# System Design & Architecture

## Architecture Overview

시스템은 중앙 집중식 **Controller**와 분산된 **Agent**들 간의 Binary Message 기반 통신 구조를 가집니다.

```mermaid
graph TD
    subgraph Controller_Node
        C[Controller] --> SS[StateStore]
        C --> MS[MetricsServer]
        C --> AS[AgentTcpSession]
    end
    
    subgraph Agent_Nodes
        A1[Agent 1]
        A2[Agent 2]
    end
    
    AS <== TCP/Binary ==> A1
    AS <== TCP/Binary ==> A2
```

## Class Diagram (Interface-based Architecture)

```mermaid
classDiagram
    class ICommandBus {
        <<interface>>
        +broadcastCommand(Message)
        +sendCommandTo(agent_id, Message)
    }
    class IAgentComm {
        <<interface>>
        +send(Message)
        +getAgentId()
        +isHealthy()
        +disconnect()
    }
    class IStateStore {
        <<interface>>
        +updateAgentState(id, payload, ts)
        +getAgentState(id)
        +getAggregateLoad()
    }
    class IMessageParser {
        <<interface>>
        +serialize(Message)
        +deserialize(data, consumed)
    }

    class Controller {
        -acceptor_
        -store_ StateStore
        -sessions_ map~uint32, IAgentComm~
        +broadcastCommand(Message)
        +sendCommandTo(agent_id, Message)
    }
    class AgentTcpSession {
        -conn_ TcpComm
        +send(Message)
        +getAgentId()
    }
    class StateStore {
        -states_ map~uint32, StatePayload~
        +updateAgentState(...)
    }
    class BinaryMessageParser {
        +serialize(Message)
        +deserialize(...)
    }
    class TcpComm {
        -socket_
        -parser_ IMessageParser
        +send(Message)
    }

    ICommandBus <|-- Controller : implements
    IAgentComm <|-- AgentTcpSession : implements
    IStateStore <|-- StateStore : implements
    IMessageParser <|-- BinaryMessageParser : implements

    Controller o-- IAgentComm : holds
    Controller *-- StateStore : uses
    AgentTcpSession *-- TcpComm : owns
    TcpComm o-- IMessageParser : uses
```

## 클래스 책임 (Responsibilities)

| Component | Responsibility |
| :--- | :--- |
| **Controller** | 에이전트 연결 관리, 정책(Policy) 트리거 로직, 명령 브로드캐스트 |
| **AgentTcpSession** | 개별 에이전트와의 세션 상태 유지, 메트릭 측정, 재시도 제어 |
| **Agent** | 시스템 상태(CPU/Temp) 수집 및 보고, 수신된 명령 실행 |
| **TcpComm** | (Core) ASIO 기반 비동기 스트림 처리 및 에러 핸들링 |
| **BinaryMessageParser** | (Core) 가변 길이 바이너리 프로토콜 직렬화/역직렬화 |

## Sequence Diagrams

### 1. Connection & Registration (Handshake)

새 에이전트가 접속하고 컨트롤러에 등록되는 과정입니다.

```mermaid
sequenceDiagram
    participant A as Agent
    participant C as Controller
    participant AS as AgentTcpSession
    participant S as StateStore
    
    A->>C: TCP Connect
    C->>AS: Create Session
    A->>AS: HELLO (Agent ID)
    AS->>C: Register Agent
    C-->>AS: OK
    AS-->>A: (Connection Established)
```

### 2. Reporting & Policy Execution

```mermaid
sequenceDiagram
    participant A as Agent
    participant AS as AgentTcpSession
    participant C as Controller
    participant S as StateStore
    
    rect rgb(240, 240, 240)
    Note over A, AS: Every 1 second
    A->>AS: HEARTBEAT / STATE
    AS->>S: updateAgentState
    end

    loop Every 2 seconds
        C->>S: Get Aggregate Load
        S-->>C: Avg Load (e.g. 85%)
        C->>C: Check Policy (Threshold > 80%)
        alt Policy Triggered
            C->>AS: Request Mode Change (Mode 1)
            AS->>A: CMD_SET_MODE
            A-->>AS: ACK
            AS-->>C: Update Last Command RC
        end
    end
```