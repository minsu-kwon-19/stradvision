# Message Sequence Diagram

This diagram illustrates the flow of a message from the Agent (Sender) to the Controller (Receiver) in the current architecture.

```mermaid
sequenceDiagram
    participant AG as Agent (Business Logic)
    participant MSG as Message / Payload
    participant TC as TcpComm (Transport)
    participant BP as BinaryMessageParser
    participant CT as Controller (Business Logic)

    Note over AG, MSG: [1. 메시지 생성 단계]
    AG->>MSG: Payload 객체 생성 (예: HelloPayload)
    AG->>MSG: Payload::serialize() -> payload_bytes
    AG->>MSG: Message 객체 생성 (Header + payload_bytes)

    Note over AG, TC: [2. 전송 요청 단계]
    AG->>TC: TcpComm::send(shared_ptr<Message>)
    TC->>BP: BinaryMessageParser::serialize(Message)
    BP-->>TC: 직렬화된 전체 바이트 (Header + Payload + CRC)
    TC->>TC: asio::async_write(socket, bytes)

    Note over TC, CT: [3. 네트워크 전송 및 수신]
    TC-->>BP: (Network) 전체 바이트 수신
    BP->>BP: CRC 검증 및 Header 파싱
    BP-->>CT: onMessage(shared_ptr<Message>) 호출

    Note over CT, MSG: [4. 페이로드 처리 단계]
    CT->>CT: MessageType 확인 (예: HELLO)
    CT->>MSG: 빈 Payload 객체 생성 (예: HelloPayload)
    CT->>MSG: Payload::deserialize(msg->payload)
    MSG-->>CT: (객체 내부 필드들이 채워짐)
    CT->>CT: 비즈니스 로직 수행 (예: 에이전트 등록)
```

## Key Components

1.  **Payload (IPayload)**: Responsible for serializing/deserializing the business-specific data fields.
2.  **Message**: A container that wraps the payload bytes with a common protocol header (ID, type, timestamp, flags).
3.  **BinaryMessageParser**: Handles the low-level framing, including header serialization, byte order conversion (Big-Endian), and CRC calculation.
4.  **TcpComm**: Manages the ASIO socket and the lifecycle of outbound/inbound byte streams.
