# Controller-Agent System

에이전트로부터 상태 정보를 수집하고 정책에 따라 명령을 내리는 컨트롤러가 존재하는 시스템

## 핵심 기능

- **비동기 통신**: `asio` 기반의 non-blocking TCP 통신
- **실시간 모니터팅**: Prometheus 형식의 메트릭 노출 (Port 9090)
- **구조화된 로깅**: 분석이 용이한 JSON 라인 로깅
- **안전성 검증**: AddressSanitizer(ASan) 및 UBSan 통합

## 의존성 및 빌드 환경

- **Compiler**: C++17 지원 컴파일러 (gcc/clang)
- **Dependencies**: `asio`, `spdlog`, `nlohmann_json`, `gtest`
- **Build System**: CMake 3.10+

## 실행 방법 (Docker 사용)

```bash
# 전체 시스템 빌드 및 실행 (Controller + Agents)
docker-compose -f docker/docker-compose.yml up --build

# ASan 성능 감시 모드로 실행
bash scripts/run_system_asan.sh
```

## 테스트 상세 명세 (Test Specifications)

각 테스트 파일은 시스템의 특정 시나리오와 안정성을 검증합니다.

| 분류 | 테스트 파일 | 실행 스크립트 | 검증 내용 |
| :--- | :--- | :--- | :--- |
| **기본 기능** | `PayloadTest.cpp` | `run_unit_tests.sh` | 모든 바이너리 페이로드의 직렬화 및 역직렬화 정밀도 검증 |
| | `MessageParserTest.cpp` | `run_message_parser_test.sh` | 바이너리 스트림 파싱 엔진의 무결성 및 헤더 처리 검증 |
| **통신 안정성** | `CommandRetryTest.cpp` | `run_command_retry_test.sh` | 에이전트로부터 ACK가 오지 않을 때 컨트롤러의 명령 재전송 로직 검증 |
| | `DuplicateMessageTest.cpp` | `run_duplicate_test.sh` | 네트워크 지연 등으로 동일한 `header_id`가 중복 수신될 때 에이전트의 무시 로직 검증 |
| **세션 관리** | `SessionTimeoutTest.cpp` | `run_unit_tests.sh` | 하트비트가 끊긴 불량 에이전트를 컨트롤러가 5초(3회 시도) 내에 감지하고 연결 해제하는지 검증 |
| **시나리오 통합** | `IntegrationTest.cpp` | `run_integration_test.sh` | 컨트롤러에 정책 주입 -> 상태 수집 -> 부하 임계치 도달 -> 모든 에이전트 모드 변경 하달 및 ACK 확인까지의 전체 사이클 검증 |

## 동적 분석 (Sanitizers)

### 실행 방법

1. **테스트 레벨 검증**

   ```bash
   # 모든 유닛/통합 테스트를 ASan 환경에서 실행
   bash scripts/run_asan_tests.sh
   ```

2. **시스템 레벨 실시간 감시**

   ```bash
   # 컨트롤러와 에이전트를 실시간 ASan 모드로 실행
   bash scripts/run_system_asan.sh
   ```
