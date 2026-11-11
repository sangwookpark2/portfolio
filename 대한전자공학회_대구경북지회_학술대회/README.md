# 🌿 센서 데이터를 활용한 스마트팜 자동제어 시스템 (Smart Farm Auto-Control System)

> **2025 대한전자공학회 대구경북지회 학술대회 포스터 발표 논문**  
> **과제 수행**: 산업통상자원부 산학융합지구조성사업 연구 결과물  
> **소속**: 대구가톨릭대학교 로봇공학과 / CRC Lab (Radio Communication & Lab)

---

## 🏆 학술대회 발표 및 논문 정보

- **논문 제목**: 센서 데이터를 활용한 스마트팜 자동제어 시스템
- **발표 학술대회**: 2025 대한전자공학회 대구경북지회 학술대회
- **저자**: 우동식, 박상욱 (대구가톨릭대학교 로봇공학과)
- **주요 성과**: 아두이노 기반 실시간 환경 데이터 수집 및 자동 제어 시퀀스 검증, IoT 확장 가능 아키텍처 설계

---

## 📌 프로젝트 개요

본 프로젝트는 스마트 농업의 핵심인 실시간 환경 데이터(온도, 습도, 토양 수분)를 기반으로 스마트팜 내부 환경을 자동으로 관리하는 아두이노(Arduino) 기반 자동제어 프로토타입 시스템입니다.

센서 데이터 수집 ➔ 환경 판단 ➔ 제어 출력의 순환 루프 알고리즘을 구축하여 장시간 연속 동작의 안정성을 확보하였으며, 향후 IoT 원격 모니터링 및 AI 기반 지능형 농업으로 확장 가능하도록 모듈형 구조로 설계되었습니다.

---

## 🛠️ 하드웨어 구성 요소 (Hardware Components)

| 구성 요소 | 역할 및 주요 기능 |
| :--- | :--- |
| **Arduino Uno** | 메인 제어부, 센서 데이터 처리 및 자동 제어 로직 수행 |
| **DHT11 온습도 센서** | 스마트팜 내부 온도 및 습도 주기적 측정 (핀: `A0`) |
| **토양 수분 센서** | 토양 수분 상태를 정량화하여 급수/환경 제어 시점 판단 (핀: `A1`) |
| **1602 I2C LCD** | 실시간 온/습도, 토양 수분 수치 및 경고 메시지 시각화 |
| **Adafruit NeoPixel RGB LED** | 12개 LED 모듈 기반 환풍/온도 상태 시각 알림 (핀: `9`) |
| **L298N DC 모터 드라이버** | 환풍기(DC 모터) 제어 (핀: `4, 5, 6, 7`) |

---

## 🏗️ 시스템 아키텍처 (System Architecture)

본 시스템은 소규모 자동화 환경부터 클라우드 연동 환경까지 확장 가능한 3-Layer 구조로 설계되었습니다.

1. **Front-end Layer (하드웨어 제어부)**: Arduino 제어 보드와 센서/액추에이터 노드가 실시간 환경 데이터 수집 및 1차 자동 제어 수행
2. **Gateway Layer (통신부)**: Wi-Fi 게이트웨이를 통해 수집된 데이터를 상위 레이어로 무선 전송
3. **Back-end Layer (상위 분석부)**: 서버/클라우드 기반 데이터 시각화, 데이터 분석 및 사용자 원격 제어 인터페이스(API) 제공

---

## 💻 메인 제어 펌웨어 소스코드 (`src/smartfarm_control.ino`)

```cpp
#include <Wire.h>               // I2C 통신 라이브러리
#include <LiquidCrystal_I2C.h>  // 1602 I2C LCD 라이브러리
#include <Adafruit_NeoPixel.h>  // 네오픽셀 RGB LED 라이브러리
#include "DHT.h"                // DHT11 온습도 센서 라이브러리

#define DHTPIN A0
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel RGB_LED = Adafruit_NeoPixel(12, 9, NEO_GRB);
LiquidCrystal_I2C lcd(0x27, 16, 2);   // I2C 접근 주소 0x27

// L298N 모터 드라이버 제어 핀 (모터 A / 모터 B)
int AA = 5; int AB = 4;
int BA = 6; int BB = 7;

int ON = 1; int OFF = 0;

// 센서 임계값 선언 및 기준값 설정
float fMax_hum = 0.0;
float fMax_temp = 0.0;
float fMin_temp = 0.0;
int Soil_moisture_reference = 0;

void setup() {
  fMax_hum  = 60.0;               // 습도 상한 기준값 (60%)
  fMax_temp = 30.0;               // 온도 상한 기준값 (30°C)
  fMin_temp = 22.0;               // 온도 하한 기준값 (22°C)
  Soil_moisture_reference = 50;   // 기준 토양 수분값 (50%)
  
  Serial.begin(9600);
  dht.begin();
   
  lcd.init();
  lcd.backlight();
  
  RGB_LED.begin();
  RGB_LED.setBrightness(200);     // RGB LED 밝기 설정
  RGB_LED.show();

  pinMode(AA, OUTPUT);
  pinMode(AB, OUTPUT);
  pinMode(BA, OUTPUT);
  pinMode(BB, OUTPUT);
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  int Soil_moisture = analogRead(A1);
  int C_Soil_moisture = map(Soil_moisture, 1023, 0, 0, 100); // 0~100% 변환
  
  String strC_Soil = int2String(C_Soil_moisture);

  // 1. 토양 수분 부족 시 경고 LCD 출력
  if(C_Soil_moisture < Soil_moisture_reference) {
    WarrningLCD();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("T : "); lcd.print((int)temp); lcd.print("C ");
    lcd.setCursor(8, 0);
    lcd.print("H : "); lcd.print((int)hum); lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.print("Soil_M : "); lcd.print(strC_Soil); lcd.print("   ");
  }

  // 2. 온도 및 습도 상태에 따른 환풍기(DC 모터) 및 네오픽셀 제어
  if((temp > fMax_temp && hum > fMax_hum) || temp > fMax_temp) {
    FanONOFF(ON);
    RGB_Color(RGB_LED.Color(0, 0, 0), 10);
  } else if(hum > fMax_hum) {
    FanONOFF(ON);
    RGB_Color(RGB_LED.Color(100, 100, 100), 10);
  } else if(temp < fMin_temp) {
    FanONOFF(OFF);
    RGB_Color(RGB_LED.Color(100, 100, 100), 10);
  } else {
    FanONOFF(OFF);
    RGB_Color(RGB_LED.Color(0, 0, 0), 10);
  }
  
  delay(200);
}

// 네오픽셀 제어 함수
void RGB_Color(float c, int wait) {
  for (int i = 0; i < RGB_LED.numPixels(); i++) {  
     RGB_LED.setPixelColor(i, c);
     RGB_LED.show();
     delay(wait);
  }
}

// LCD 표시 자릿수 정렬 함수
String int2String(int val) {
  String sval = "";
  int val2 = (val/10)%10;   // 10의 자리
  int val3 = (val/100)%10;  // 100의 자리
  
  if(val3 >= 1) {
    sval = String(val);
  } else {
     if(val2 >= 1) sval = " "+String(val);
     else sval = "  "+String(val);
  }
  return sval;
}

// 수분 경고 메세지 출력 함수
void WarrningLCD(){ 
    lcd.clear();
    delay(200);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Not enough water");
    lcd.setCursor(0, 1);
    lcd.print("supply water!!");
    delay(200);
}

// 모터 드라이버(L298N) 제어 함수
void FanONOFF(int OnOff) {
  if(OnOff == 1) {
      digitalWrite(AA, HIGH); digitalWrite(AB, LOW);
      digitalWrite(BA, HIGH); digitalWrite(BB, LOW);
  } else {
      digitalWrite(AA, LOW); digitalWrite(AB, LOW);
      digitalWrite(BA, LOW); digitalWrite(BB, LOW);
  }
}
