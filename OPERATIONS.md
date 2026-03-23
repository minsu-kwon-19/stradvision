# Operations Guide (Logging & Monitoring)

## 구조화된 로깅 (JSON Line Format)
모든 로그는 즉시 파싱 가능한 JSON 형태로 출력됩니다.

**포맷 예시:**
```json
{"time": "2026-03-23T19:42:15.123+0900", "level": "info", "message": "Accepted new connection"}
{"time": "2026-03-23T19:42:16.456+0900", "level": "warn", "message": "Agent 3 is unhealthy, dropping session"}
```

## 메트릭 모니터링 (Prometheus)

**노출 메트릭:**
*   `active_connections`: 현재 연결된 에이전트 수
*   `command_failures_total`: 명령 전달 실패 누적 횟수
*   `average_rtt_ms`: 평균 왕복 지연 시간

**확인 방법:**

```bash
curl http://localhost:9090/metrics
```