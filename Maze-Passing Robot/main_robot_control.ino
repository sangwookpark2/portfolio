/*
 * =====================================================================================
 * 프로젝트명: 초음파 센서 기반 자율주행 및 RC 겸용 미로통과 로봇
 * 작성자: 빠르조 (박상욱, 박민상, 우동식, 강성묵)
 * 주요 기능: 
 *   - 5개 초음파 센서를 활용한 장애물 회피 및 벽 중심 정렬(alignToCenter)
 *   - L298N 모터 드라이버 및 탱크 섀시 기반 차동 회전 최적화
 *   - RC PWM 신호 0점 보정 (Deadband Filter) 및 gradualChange() 가감속 속도 제어
 *   - 버튼 스위치를 통한 자율주행 <-> RC 수동 모드 실시간 전환
 * =====================================================================================
 */

// -------------------------------------------------------------------------------------
// 1. 핀 배치 및 변수 정의
// -------------------------------------------------------------------------------------

// 모터 제어 핀 (L298N)
const int motor1Pin1 = 22;
const int motor1Pin2 = 23;
const int motor2Pin1 = 24;
const int motor2Pin2 = 25;

// PWM 속도 제어 핀 (ENA, ENB)
const int motor1Enable = 5;
const int motor2Enable = 6;

// 초음파 센서 핀 (5개 센서 구성)
const int trigFront  = 30; const int echoFront  = 31;
const int trigLeft   = 32; const int echoLeft   = 33;
const int trigRight  = 34; const int echoRight  = 35;
const int trigFLeft  = 36; const int echoFLeft  = 37; // 좌측 대각선 센서
const int trigFRight = 38; const int echoFRight = 39; // 우측 대각선 센서

// 모드 전환 버튼 핀
const int buttonPin = 2;
bool autoMode = true;
bool lastButtonState = HIGH;
bool currentButtonState;

// RC 수신기 입력 핀 및 입력 변수
const int throttlePin = 3;
const int steeringPin = 4;

// 모터 속도 관련 변수 (가감속 제어용)
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
const int accelStep = 40;     // 가감속 스텝 크기 (점진적 속도 변경 폭)
const int minTurnSpeed = 60;  // 회전 시 최소 유지 속도 보정값

// -------------------------------------------------------------------------------------
// 2. 초기화 (setup)
// -------------------------------------------------------------------------------------
void setup() {
    // 모터 핀 출력 설정
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);
    pinMode(motor1Enable, OUTPUT);
    pinMode(motor2Enable, OUTPUT);

    // 초음파 센서 핀 설정
    pinMode(trigFront, OUTPUT);  pinMode(echoFront, INPUT);
    pinMode(trigLeft, OUTPUT);   pinMode(echoLeft, INPUT);
    pinMode(trigRight, OUTPUT);  pinMode(echoRight, INPUT);
    pinMode(trigFLeft, OUTPUT);  pinMode(echoFLeft, INPUT);
    pinMode(trigFRight, OUTPUT); pinMode(echoFRight, INPUT);

    // 버튼 및 RC 핀 설정
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(throttlePin, INPUT);
    pinMode(steeringPin, INPUT);

    Serial.begin(9600);
    Serial.println("System Initialized! Ready to run.");
}

// -------------------------------------------------------------------------------------
// 3. 메인 루프 (loop)
// -------------------------------------------------------------------------------------
void loop() {
    checkModeButton();
    if (autoMode) {
        runAutonomousMode();
    } else {
        runRCMode();
    }
}

// -------------------------------------------------------------------------------------
// 4. 모드 전환 및 메인 동작 함수
// -------------------------------------------------------------------------------------
void checkModeButton() {
    currentButtonState = digitalRead(buttonPin);
    if (lastButtonState == HIGH && currentButtonState == LOW) {
        autoMode = !autoMode;
        Serial.print("Mode Changed: ");
        Serial.println(autoMode ? "Autonomous Mode" : "RC Manual Mode");
        delay(500); // 디바운싱 지연
    }
    lastButtonState = currentButtonState;
}

// [자율주행 모드]
void runAutonomousMode() {
    long front  = measureDistance(trigFront, echoFront);
    long left   = measureDistance(trigLeft, echoLeft);
    long right  = measureDistance(trigRight, echoRight);
    long fLeft  = measureDistance(trigFLeft, echoFLeft);
    long fRight = measureDistance(trigFRight, echoFRight);

    Serial.print("Front: "); Serial.print(front);
    Serial.print(" | Left: "); Serial.print(left);
    Serial.print(" | Right: "); Serial.print(right);
    Serial.print(" | FLeft: "); Serial.print(fLeft);
    Serial.print(" | FRight: "); Serial.println(fRight);

    // 1. 정면에 장애물이 없으면 벽 중심 정렬을 유지하며 전진
    if (front > 30) {
        moveForwardWithAlign();
    }
    // 2. 정면에 장애물이 있고 한쪽에만 공간이 있는 경우
    else if (left > 30 && right <= 30) {
        turnRight();
    } 
    else if (right > 30 && left <= 30) {
        turnLeft();
    }
    // 3. 정면과 양 측면 모두 막혀 대각선 센서 판별이 필요한 경우
    else if (left > 30 && right > 30) {
        if (fLeft < 30 && fRight < 30) {
            if (front > 30) {
                moveForwardWithAlign();
            } else {
                stopMotors();
            }
        } 
        else if (fLeft < 30 && right > 30 && front > 30) {
            turnRight();
        } 
        else if (fRight < 30 && left > 30 && front > 30) {
            turnLeft();
        } 
        else {
            if (left > right) turnLeft();
            else turnRight();
        }
    }
    // 4. 근접 장애물 정밀 회회전
    else if (front <= 30) {
        if (left > right) {
            while (measureDistance(trigFront, echoFront) <= 30) {
                turnRight();
            }
        } else {
            while (measureDistance(trigFront, echoFront) <= 30) {
                turnLeft();
            }
        }
        delay(100);
        moveForwardWithAlign();
    } 
    else {
        stopMotors();
    }
}

// [RC 수동 조종 모드]
void runRCMode() {
    static int lastThrottle = 1400;
    static int lastSteering = 1400;

    int throttleRaw = pulseIn(throttlePin, HIGH, 50000);
    int steeringRaw = pulseIn(steeringPin, HIGH, 50000);

    if (throttleRaw > 900 && throttleRaw < 1800) lastThrottle = throttleRaw;
    if (steeringRaw > 900 && steeringRaw < 1800) lastSteering = steeringRaw;

    int throttleVal = map(lastThrottle, 1000, 1800, -255, 255);
    int steeringVal = map(lastSteering, 1000, 1800, -255, 255);

    // 0점/중립 범위 보정 (Deadband Filter: 30 이하 입력 무시)
    if (abs(throttleVal) < 30) throttleVal = 0;
    if (abs(steeringVal) < 30) steeringVal = 0;

    int targetLeftSpeed  = throttleVal + steeringVal;
    int targetRightSpeed = throttleVal - steeringVal;

    // 차동 회전 시 바퀴 최소 속도 보정
    if (steeringVal < 0) { // 좌회전 시 우측 바퀴 보정
        if (abs(targetRightSpeed) < minTurnSpeed) {
            targetRightSpeed = (targetRightSpeed >= 0) ? minTurnSpeed : -minTurnSpeed;
        }
    } 
    else if (steeringVal > 0) { // 우회전 시 좌측 바퀴 보정
        if (abs(targetLeftSpeed) < minTurnSpeed) {
            targetLeftSpeed = (targetLeftSpeed >= 0) ? minTurnSpeed : -minTurnSpeed;
        }
    }

    // 가감속 점진적 적용
    currentLeftSpeed  = gradualChange(currentLeftSpeed, targetLeftSpeed, accelStep);
    currentRightSpeed = gradualChange(currentRightSpeed, targetRightSpeed, accelStep);

    setMotorSpeed(currentLeftSpeed, currentRightSpeed);
    delay(100);
}

// -------------------------------------------------------------------------------------
// 5. 모터 제어 및 유틸리티 함수
// -------------------------------------------------------------------------------------

// 초음파 센서 거리 측정 함수 (cm 단위 반환)
long measureDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);
    if (duration == 0) return 999; // 신호 미수신 시 예외값
    return duration * 0.0344 / 2;
}

// 모터 속도 및 방향 설정 함수
void setMotorSpeed(int left, int right) {
    left  = constrain(left, -255, 255);
    right = constrain(right, -255, 255);

    // 좌측 모터 제어
    if (left == 0) {
        digitalWrite(motor1Pin1, LOW);
        digitalWrite(motor1Pin2, LOW);
    } else if (left > 0) {
        digitalWrite(motor1Pin1, HIGH);
        digitalWrite(motor1Pin2, LOW);
    } else {
        digitalWrite(motor1Pin1, LOW);
        digitalWrite(motor1Pin2, HIGH);
        left = -left;
    }

    // 우측 모터 제어
    if (right == 0) {
        digitalWrite(motor2Pin1, LOW);
        digitalWrite(motor2Pin2, LOW);
    } else if (right > 0) {
        digitalWrite(motor2Pin1, HIGH);
        digitalWrite(motor2Pin2, LOW);
    } else {
        digitalWrite(motor2Pin1, LOW);
        digitalWrite(motor2Pin2, HIGH);
        right = -right;
    }

    analogWrite(motor1Enable, left);
    analogWrite(motor2Enable, right);
}

// 주행 기본 동작 함수
void moveForward() {
    setMotorSpeed(150, 140);
}

void turnLeft() {
    stopMotors();
    delay(500);
    setMotorSpeed(-160, 150);
    delay(500);
    stopMotors();
}

void turnRight() {
    stopMotors();
    delay(500);
    setMotorSpeed(160, -150);
    delay(500);
    stopMotors();
}

void stopMotors() {
    analogWrite(motor1Enable, 0);
    analogWrite(motor2Enable, 0);
}

// 벽 중심 정렬 함수
void alignToCenter() {
    long left  = measureDistance(trigLeft, echoLeft);
    long right = measureDistance(trigRight, echoRight);
    long diff  = left - right;

    if (abs(diff) > 5) {
        if (diff > 0) {
            setMotorSpeed(80, 80);
            delay(120);
        } else {
            setMotorSpeed(-80, 80);
            delay(120);
        }
        stopMotors();
    }
}

// 중심 정렬을 포함한 직진
void moveForwardWithAlign() {
    alignToCenter();
    setMotorSpeed(150, 140);
    delay(100);
    stopMotors();
}

// 속도 점진 변경 (가감속 제어) 함수
int gradualChange(int current, int target, int step) {
    if (current < target) {
        current += step;
        if (current > target) current = target;
    } else if (current > target) {
        current -= step;
        if (current < target) current = target;
    }
    return current;
}
