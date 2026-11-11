# 🤖 장애물 통과 및 미로 탐색 자율주행/RC 로봇 (Obstacle-Avoiding & Maze-Passing Robot)

> **대구가톨릭대학교 로봇공학과 캡스톤 디자인 / 무선조종 및 자율주행 로봇 프로젝트**  
> 초음파 센서 배열과 아두이노(Arduino Mega 2560)를 활용하여 장애물 감지, 벽 중심 정렬, 가감속 제어 및 RC/자율주행 모드 스위칭을 구현한 소형 주행 로봇입니다.

---

## 📌 1. 프로젝트 개요 (Overview)
- **팀명:** 빠르조
- **팀원:** 박상욱(팀장, 하드웨어/소프트웨어 총괄 및 제작), 박민상, 우동식, 강성묵
- **주요 목표:** 
  - 초음파 센서 실시간 거리 데이터를 기반으로 한 미로 통과 및 장애물 회피 알고리즘 구현
  - L298N 모터 드라이버 및 탱크 섀시 기반의 차동 회전(Differential Steering) 최적화
  - 가감속 속도 제어 알고리즘 및 0점 보정을 통한 안정적인 RC 수동 제어 구현
  - 스위치 버튼을 통한 자율주행 모드 ↔ RC 수동 모드 원터치 전환

---

## 🛠️ 2. 하드웨어 구성 (Hardware Setup)

| 구 분 | 상세 사양 및 부품 | 비 고 |
| :--- | :--- | :--- |
| **Main Controller** | Arduino Mega 2560 | 실시간 센서 데이터 처리 및 PWM 제어 |
| **Motor Driver** | L298N 모터 드라이버 | PWM 핀 5, 6 사용 / DC 모터 2개 구동 |
| **Chassis & Actuator**| 탱크형 궤도 섀시 + DC 모터 2개 | 차동 구동 제어 |
| **Sensors** | HC-SR04 초음파 센서 5개 | 전방, 좌/우측, 좌/우 대각선 배치 |
| **Power System** | 18650 리튬이온 배터리 (3000mAh × 3, 병렬) | 12V 전원 공급 |
| **RC Controller** | RC 수신기 (Throttle Pin 3, Steering Pin 4) | PWM 신호 수신 |
| **Structure** | 3D 프린팅 프레임 & 센서 마운트 | 로봇 프레임 경량화 및 내구성 확보 |

---

## 💡 3. 핵심 알고리즘 및 기능 (Key Features)

### 1) 자율주행 & 미로 통과 알고리즘
- **멀티 초음파 센서 거리 감지:** 전방 및 대각선, 측면 센서를 통해 가장 개방된 방향을 실시간 판단
- **벽 중심 정렬 (`alignToCenter`):** 좌/우 벽과의 거리를 비교하여 통로 중심을 유지하며 직진 주행
- **대각선 센서 장애물 판단:** 좁은 미로 및 협소 공간에서 전방/측면 감지 한계를 보완하여 부드러운 회피 방향 결정

### 2) RC 제어 및 주행 최적화
- **0점/중립 범위 보정 (Deadband Filter):** RC 수신기 PWM 신호의 미세한 노이즈를 제거하여 정지 시 모터 떨림 완벽 방지 (입력 오차 범위 무시)
- **가감속 제어 알고리즘 (`gradualChange`):** 목표 속도까지 `accelStep` 기반으로 점진적 속도 변경을 적용하여 급출발/급정지로 인한 전복 및 기계적 충격 최소화
- **차동 회전 최적화 (Minimum Turn Speed Compensation):** 회전 시 비회전측 바퀴의 최소 속도를 보정하여 정지 현상을 막고 회전 반경 최소화

---

## 📂 4. 소스 코드 구조 (Code Structure)

주요 아두이노 제어 함수 구조:

```cpp
void setup()                // 핀 모드 설정 및 초음파/RC 수신기 초기화
void loop()                 // 모드 전환 버튼 감지 및 선택된 모드 실행 (runAutonomousMode / runRCMode)

// [자율주행 모드]
void runAutonomousMode()    // 5개 초음파 센서 거리 측정 및 장애물 판단 로직
void moveForwardWithAlign() // 중심 정렬(alignToCenter)을 포함한 직진 주행
void alignToCenter()        // 좌/우 벽 거리 차 보정
void turnLeft() / turnRight() // 장애물 회피 회전 동작

// [RC 수동 모드]
void runRCMode()            // PulseIn을 이용한 RC PWM 신호 파싱
int gradualChange()         // 속도 변화 스텝 적용 (가감속 제어)
void setMotorSpeed()        // 좌/우 모터 독립 속도 제어 및 방향 결정
