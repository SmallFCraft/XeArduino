// Arduino UNO + L298N + 3 cam bien line + HC-SR04
// Cam bien line: 1 = den (line), 0 = trang (nen)
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 4); // RX, TX

// ===== Ket noi L298N =====
const int ENA = 5;   // PWM motor trai
const int IN1 = 6;   // IN1 (trai)
const int IN2 = 7;   // IN2 (trai)

const int IN3 = 8;   // IN3 (phai)
const int IN4 = 9;   // IN4 (phai)
const int ENB = 10;  // PWM motor phai

// ===== Cam bien line =====
const int L_S = A0;  // Trai
const int C_S = A1;  // Giua
const int R_S = A2;  // Phai

// ===== HC-SR04 Ultrasonic Sensor =====
const int TRIG = 11;
const int ECHO = 12;

double cm = 0;
double dura = 0;

// ========== PID CONFIG ==========
// Khai bao thong so PID
float Kp = 28; 
float Ki = 0.05;
float Kd = 4;

float error = 0, P = 0, I = 0, D = 0, PIDvalue = 0;
float previousError = 0;

// Bien cho thuat toan PID moi (co history)
int tinhieutrai = -1;
int tinhieuphai = -1;
int temp = -1;
unsigned long lastTimePID = 0;

const int BASE_SPEED = 105; 
const int MAX_SPEED = 130;
const int AVOID_SPEED = 140; 
const int MANUAL_SPEED = 160; // Toc do chay Manual (Manh hon de tranh ma sat) 

// ========== Trang thai ==========
bool started = false;
bool isAutoMode = true; // Mac dinh chay Auto (Do line)

// ========== Ham dieu khien motor ==========
void setLeft(int speed) {
  if (speed > 255) speed = 255;
  if (speed < -255) speed = -255;

  if (speed > 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, speed);
  } else if (speed < 0) {
    speed = -speed;
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, speed);
  } else {
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

void setRight(int speed) {
  if (speed > 255) speed = 255;
  if (speed < -255) speed = -255;

  if (speed > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speed);
  } else if (speed < 0) {
    speed = -speed;
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, speed);
  } else {
    analogWrite(ENB, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void stopCar() {
  setLeft(0);
  setRight(0);
}

void distanceCar() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  dura = pulseIn(ECHO, HIGH, 30000); // Timeout 30ms
  
  if (dura == 0) {
    cm = 999; // Khong co vat can hoac qua xa
  } else {
    cm = dura / 29 / 2;
  }

  Serial.print("Distance: ");
  Serial.print(cm);
  Serial.println("cm");
}

void avoidObstacle() {
  Serial.println("Avoid Obstacle Start");

  // 1. Lui lai
  setLeft(-AVOID_SPEED);
  setRight(-AVOID_SPEED);
  delay(200);
  stopCar();
  delay(200);

  // 2. Xoay Phai
  setLeft(AVOID_SPEED);
  setRight(-AVOID_SPEED);
  delay(250);
  stopCar();
  delay(200);

  // 3. Di Thang
  setLeft(AVOID_SPEED);
  setRight(AVOID_SPEED);
  delay(500);
  stopCar();
  delay(200);

  // 4. Xoay Trai
  setLeft(-AVOID_SPEED);
  setRight(AVOID_SPEED);
  delay(250);
  stopCar();
  delay(200);

  // 5. Di Thang
  setLeft(AVOID_SPEED);
  setRight(AVOID_SPEED);
  delay(500);
  stopCar();
  delay(200);

  // 6. Xoay Trai
  setLeft(-AVOID_SPEED);
  setRight(AVOID_SPEED);
  delay(250);
  stopCar();
  delay(200);

  // 7. Di Thang va Tim Line
  setLeft(AVOID_SPEED);
  setRight(AVOID_SPEED);
  
  while (digitalRead(L_S) == 0) {
    // Cho den khi mat trai cham line
    // Co the them timeout de tranh treo
  }
  stopCar();
  
  // 8. Can chinh huong (Xoay Phai de thang line)
  // Vi robot dang di tu ben phai ve, dau xe huong sang trai -> Can xoay phai
  setLeft(AVOID_SPEED);
  setRight(-AVOID_SPEED);
  delay(300); // Thoi gian xoay phai de thang line (can chinh thuc te)
  stopCar();
  delay(200);

  Serial.println("Line Found! Resume...");

  // RESET PID STATE
  error = 0;
  previousError = 0;
  I = 0;
  PIDvalue = 0;
  tinhieutrai = -1;
  tinhieuphai = -1;
  lastTimePID = millis(); // Reset thoi gian de tranh dt qua lon
}

// ==========================================

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600); // Giao tiep voi ESP8266

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(L_S, INPUT);
  pinMode(C_S, INPUT);
  pinMode(R_S, INPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stopCar();
  delay(1000);  // Cho motor on dinh truoc khi bat dau
}

void loop() {
  // 1. Doc lenh tu ESP8266
  if (mySerial.available()) {
    char cmd = mySerial.read();
    Serial.print("CMD: "); Serial.println(cmd);
    
    if (cmd == 'A') {
      isAutoMode = true;
      stopCar();
      Serial.println("Mode: AUTO");
    } 
    else if (cmd == 'M') {
      isAutoMode = false;
      stopCar();
      Serial.println("Mode: MANUAL");
    }
    
    // Dieu khien Manual
    if (!isAutoMode) {
      switch (cmd) {
        case 'F': setLeft(MANUAL_SPEED); setRight(MANUAL_SPEED); break; // Tien
        case 'B': setLeft(-MANUAL_SPEED); setRight(-MANUAL_SPEED); break; // Lui
        case 'L': setLeft(-MANUAL_SPEED); setRight(MANUAL_SPEED); break; // Trai
        case 'R': setLeft(MANUAL_SPEED); setRight(-MANUAL_SPEED); break; // Phai
        case 'S': stopCar(); break; // Dung
      }
    }
  }

  // Neu dang o Manual Mode -> Bo qua phan do line
  if (!isAutoMode) {
    return;
  }

  // ===== AUTO MODE (Do Line + Tranh Vat Can) =====
  int L = digitalRead(L_S);
  int C = digitalRead(C_S);
  int R = digitalRead(R_S);

  distanceCar();
  // Chi chay khi phat hien line
  if (!started) {
    stopCar();
    // Neu thay line -> bat dau chay
    if (L == 1 || C == 1 || R == 1) {
      started = true;
      Serial.println("Line detected! Starting...");
    }
    delay(10);
    return;
  }

  if (cm < 15) {
    Serial.println("Phat hien vat can!");
    stopCar();
    delay(1000); // Dung 1s truoc khi tranh
    avoidObstacle();
    return; // Quay lai dau vong loop de doc lai cam bien
  }
  // ===== TU DAY TRO XUONG: MODE_LINE (PID with HISTORY) =====

  // 1. Xac dinh gia tri loi (Error) dua tren trang thai hien tai va qua khu
  // Logic: 1 = Line (Den), 0 = Nen (Trang)
  
  if (L == 0 && C == 0 && R == 0 && tinhieutrai == 1) {
    error = -4; // GIAM TU -6 -> -4: Mat line lech trai -> quay phai nhe hon
  } 
  else if (L == 0 && C == 0 && R == 0 && tinhieuphai == 1) {
    error = 4;  // GIAM TU 6 -> 4: Mat line lech phai -> quay trai nhe hon
  }
  else if (L == 1 && C == 1 && R == 1) {
    stopCar();
    error = 0; // Ngang line / Nga tu -> Di thang
  }
  else if (L == 0 && C == 1 && R == 1) {
    error = 5; // GIAM TU 8 -> 5: Cua phai gat -> quay trai nhe hon
    tinhieuphai = 1;
    tinhieutrai = 0; // Xoa co trai
    temp = 0;
  }
  else if (L == 1 && C == 1 && R == 0) {
    error = -5; // GIAM TU -8 -> -5: Cua trai gat -> quay phai nhe hon
    tinhieutrai = 1;
    tinhieuphai = 0; // Xoa co phai
    temp = 0;
  }
  else if (L == 1 && C == 0 && R == 0) {
    error = -2; // Lech trai
    tinhieutrai = 1;
    tinhieuphai = 0; // Xoa co phai
    temp = 0;
  }
  else if (L == 0 && C == 0 && R == 1) {
    error = 2; // Lech phai
    tinhieuphai = 1;
    tinhieutrai = 0; // Xoa co trai
    temp = 0;
  }
  else if (L == 0 && C == 1 && R == 0) {
    error = 0; // Giua line
    tinhieutrai = 0;
    tinhieuphai = 0;
    temp = 1;
  }
  else if (L == 0 && C == 0 && R == 0 && temp == 1) {
    error = 0; 
  }

  // 2. Tinh toan PID
  unsigned long now = millis();
  double dt = (now - lastTimePID) / 1000.0;
  if (dt == 0) dt = 0.001; // Tranh chia 0
  lastTimePID = now;

  P = error;
  I += error * dt;
  D = (error - previousError) / dt;
  
  // Gioi han I
  if (I > 60) I = 60;
  if (I < -60) I = -60;

  PIDvalue = (Kp * P) + (Ki * I) + (Kd * D);
  previousError = error;
  
  // Gioi han PID
  if (PIDvalue > 250) PIDvalue = 250;
  if (PIDvalue < -255) PIDvalue = -255;

  // 3. Dieu khien Motor
  // Left Motor = Base + PID
  // Right Motor = Base - PID
  
  int leftMotorSpeed = BASE_SPEED + PIDvalue;
  int rightMotorSpeed = BASE_SPEED - PIDvalue;

  // Constrain toc do
  if (leftMotorSpeed > MAX_SPEED) leftMotorSpeed = MAX_SPEED;
  if (leftMotorSpeed < -MAX_SPEED) leftMotorSpeed = -MAX_SPEED;
  
  if (rightMotorSpeed > MAX_SPEED) rightMotorSpeed = MAX_SPEED;
  if (rightMotorSpeed < -MAX_SPEED) rightMotorSpeed = -MAX_SPEED;

  setLeft(leftMotorSpeed);
  setRight(rightMotorSpeed);

  delay(5);
}
