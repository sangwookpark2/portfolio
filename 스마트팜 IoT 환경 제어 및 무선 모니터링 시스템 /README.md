# 🌿(진행중) ESP32 & LoRa 기반 스마트팜 multi-channel 환경 모니터링 및 제어 시스템(26.09~11.17)


ESP32 마이크로컨트롤러와 **LoRa (E32-900T20D / 915MHz)** 장거리 무선 통신 모듈을 활용하여 농가 및 온실 환경의 7종 센서 데이터를 무선으로 수집하고, 릴레이 기반 액추에이터(수중 펌프, 환기 팬)를 자동/수동 제어하는 다채널(Multi-Channel) 스마트팜 IoT 시스템입니다.

---

## 📌 주요 특징 (Key Features)

* **7종 환경 센서 통합 모니터링:** 대기 CO2, 온습도, 토양 수분, 조도, 수온, 수질(TDS), 무접점 수위 감지
* **LoRa 915MHz 장거리 무선 통신:** 장애물이 있는 야외/온실 환경에서도 100m 이상 안정적인 RF 데이터 송수신
* **다채널(Multi-Channel) 지원:** 채널/ID 할당을 통해 1대의 수신기(Rx)에서 최대 4개 이상 구역(Tx)의 데이터를 통합 모니터링
* **안전 전원 회로 설계:** 5V 7A SMPS 메인 전원과 TP4056 + 18650 배터리 기반 비상 전원 이중화
* **웹 대시보드 관제:** 수신기 ESP32의 Wi-Fi 웹서버를 통한 실시간 수치 확인 및 릴레이 수동 제어

---

## 🛠️ 기술 스택 (Tech Stack)

* **Hardware:** ESP32 DevKitC WROOM-32D, LoRa E32-900T20D (915MHz), Mean Well 5V 7A SMPS, 5V Relay Modules
* **Firmware & Language:** C/C++ (Arduino IDE), ESP32 Hardware Serial & SoftwareSerial
* **Protocols:** LoRa RF Communication, I2C, UART, OneWire, ADC
* **Sensors:** MH-Z19C (CO2), AM2302/DHT22 (Temp/Humid), SEN0193 (Soil Moisture), GY-302/BH1750 (Light), DS18B20 (Water Temp), SZH-TSM002 (TDS), SEN0204 (Water Level)

---

## 📐 시스템 아키텍처 (System Architecture)

<img width="3856" height="2514" alt="diagram" src="https://github.com/user-attachments/assets/8346d787-87d0-48de-949e-453a2272caa6" />

[ 송신기 (Tx - 현장 1~4구역) ]
  ├── 센서 7종 수집 (CO2, 온습도, 토양수분, 조도, 수온, 수질, 수위)
  ├── ESP32 MCU 데이터 연산 및 릴레이 제어 (수중 펌프 2개, 5V 팬 2개)
  └── LoRa E32-900T20D 모듈 (915MHz RF 무선 데이터 송신)
                                   │
                                   │ (915MHz LoRa RF 장거리 무선 통신)
                                   ▼
[ 수신기 (Rx - 실내 관제) ]
  ├── LoRa E32-900T20D 모듈 (구역별 데이터 무선 수신)
  ├── ESP32 Wi-Fi AP / WebServer 구동
  └── 스마트폰 & PC 웹 브라우저 실시간 대시보드 표시 및 원격 제어



### 🔌 송신기 (Tx) ESP32 핀맵 (LoRa 기준)

| 구분 | 부품 / 센서명 | ESP32 GPIO 핀 | 비고 |
| :--- | :--- | :--- | :--- |
| **무선 통신** | LoRa E32-900T20D | `GPIO 4` (RX), `GPIO 5` (TX) | 915MHz UART 통신 |
| **대기 이산화탄소** | MH-Z19C | `GPIO 16` (RX2), `GPIO 17` (TX2) | 하드웨어 시리얼2 (5V 전원 필수) |
| **대기 온습도** | AM2302 (DHT22) | `GPIO 13` | 10KΩ 풀업 저항 연결 |
| **수조 수온** | SEN050007 (DS18B20) | `GPIO 14` | 방수형, 10KΩ 풀업 저항 연결 |
| **대기 조도** | GY-302 (BH1750) | `GPIO 21` (SDA), `GPIO 22` (SCL) | I2C 통신 |
| **토양 수분** | SEN0193 | `GPIO 32` | 정전식 부식방지 (ADC) |
| **수질 (TDS)** | SZH-TSM002 | `GPIO 33` | 양액 농도 측정 (ADC) |
| **수조 수위** | SEN0204 | `GPIO 34` | 무접점 디지털 입력 |
| **릴레이 제어** | 5V 1채널 릴레이 (4개) | `GPIO 25`, `GPIO 26`, `GPIO 27`, `GPIO 12` | 펌프 2개, 팬 2개 제어 |

---

### 📡 수신기 (Rx) ESP32 핀맵

| 부품명 | ESP32 GPIO 핀 | 비고 |
| :--- | :--- | :--- |
| LoRa E32-900T20D (RX) | `GPIO 16` (RX2) | LoRa ➔ ESP32 수신 |
| LoRa E32-900T20D (TX) | `GPIO 17` (TX2) | ESP32 ➔ LoRa 송신 |
| 전원 (USB 5V 포트) | - | 5V 2A 아답터 상시 연결 |


⚡ 전원 구성 (Power Supply)
송신기(Tx): Mean Well 5V 7A SMPS (LRS-35-5) 메인 전원 공급

비상 전원: TP4056 충전 모듈 + 18650 Li-ion 배터리 + DC-DC Boost converter (SMPS 정전 시 백업)

수신기(Rx): 5V 2A 벽걸이형 DC 아답터 (ESP32 USB 상시 전원)





