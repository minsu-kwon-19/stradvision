# Software Architecture UML

이 문서는 프로젝트의 전체적인 클래스 구조와 객체 지향적 의존 관계를 설명하는 UML 모델입니다. 
Markdown 내장 **Mermaid** 문법을 사용하여 작성되었습니다. (VS Code 등에서 Mermaid 익스텐션이나 GitHub를 통해 시각적으로 렌더링해서 볼 수 있습니다.)

## 1. 종합 클래스 다이어그램 (Class Diagram)

아래 다이어그램은 `Core`, `Controller`, `Agent` 계층별 인터페이스 상속 구조와 의존성(Composition/Has-A)을 나타냅니다.

```mermaid
classDiagram
    %% Core Interfaces
    namespace Core_Interfaces {
        class IAgentConnection {
            <<interface>>
            +send(std::shared_ptr~Message~ msg)*
            +getAgentId() const*
            +isHealthy() const*
            +disconnect()*
        }
        class ICommandBus {
            <<interface>>
            +broadcastCommand(msg)*
            +sendCommandTo(agent_id, msg)*
        }
        class IStateStore {
            <<interface>>
            +updateAgentState()*
            +getAggregateLoad()*
        }
    }

    %% Core Protocol
    namespace Core_Protocol {
        class IMessageParser {
            <<interface>>
            +serialize(Message msg)*
            +deserialize(data, bytes_consumed)*
        }
        class BinaryMessageParser {
            +serialize(Message msg)
            +deserialize(data, bytes_consumed)
        }
        class Message {
            <<DTO>>
            +Header header
            +vector~uint8_t~ payload
            +uint32_t crc32
        }
    }

    %% Core Net
    namespace Core_Net {
        class TcpComm {
            -protocol_ : IMessageParser
            +create(ioc, socket, protocol)$
            +start()
            +send(msg)
            +disconnect()
        }
    }

    %% Controller
    namespace Controller {
        class ControllerClass {
            -sessions_ : map~AgentSession~
            -store_ : StateStore
            +broadcastCommand(msg)
        }
        class AgentSession {
            -conn_ : TcpComm
            -id_ : uint32_t
            -last_heartbeat_
            +send(msg)
            +isHealthy()
        }
        class StateStore {
            +updateAgentState()
            +getAggregateLoad()
        }
    }

    %% Agent
    namespace Agent {
        class AgentClass {
            -conn_ : TcpComm
            -agent_id_
            +sendHello()
            +startReporting()
        }
    }

    %% Inheritance (IS-A)
    IAgentConnection <|.. AgentSession : implements
    ICommandBus <|.. ControllerClass : implements
    IStateStore <|.. StateStore : implements
    IMessageParser <|.. BinaryMessageParser : implements

    %% Composition & Aggregation (HAS-A)
    TcpComm *-- IMessageParser : 의존성 주입 (DI)
    AgentSession *-- TcpComm : 통신 객체 소유
    AgentClass *-- TcpComm : 통신 객체 소유
    
    ControllerClass *-- AgentSession : 논리적 세션 관리
    ControllerClass *-- StateStore : 상태 저장소 관리

    %% Data Dependency
    BinaryMessageParser ..> Message : 파싱/생성
```

## 2. 아키텍처 주요 포인트 설명

* **Core 계층 (`Core_Net`, `Core_Protocol`)**: 오직 소켓 비동기 통신과 바이트 배열 파싱만 담당하는 하위 수준의 부품들입니다.
* **Controller 계층**: `TcpComm`이라는 통신 부품을 주입받은 `AgentSession`을 통해 상태와 비즈니스 로직(ID, Heartbeat 확인 등)을 처리합니다.
* **비즈니스 로직과 통신의 완벽한 분리**: 인터페이스(`IAgentConnection`, `ICommandBus` 등)를 통해 구현체 간 강한 결합을 피하면서 높은 수준의 객체지향 설계(SOLID)를 유지하고 있습니다.
* **의존성 역전 원칙(DIP)**: 통신 객체(`TcpComm`)는 특정 파싱 규칙에 종속되지 않고 언제든 다른 파서로 갈아끼울 수 있도록 추상화된 `IMessageParser`에만 의존합니다.
