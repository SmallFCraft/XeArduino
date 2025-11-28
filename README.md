# XeArduino - Line Following Robot with WiFi Control

Xe robot dò line tự động với khả năng điều khiển từ xa qua WiFi, sử dụng Arduino UNO và ESP8266.

## 🎯 Tính năng

- **Chế độ AUTO**: Dò line tự động với thuật toán PID
- **Chế độ MANUAL**: Điều khiển bằng joystick ảo qua giao diện web
- **Tránh vật cản**: Sử dụng cảm biến siêu âm HC-SR04
- **Điều khiển WiFi**: ESP8266 phát WiFi, điều khiển qua trình duyệt

## 🔧 Phần cứng

| Linh kiện     | Số lượng | Mô tả               |
| ------------- | -------- | ------------------- |
| Arduino UNO   | 1        | Vi điều khiển chính |
| ESP8266       | 1        | Module WiFi         |
| L298N         | 1        | Driver motor        |
| Cảm biến line | 3        | Trái, Giữa, Phải    |
| HC-SR04       | 1        | Cảm biến siêu âm    |
| Motor DC      | 2        | Động cơ bánh xe     |

## 📌 Sơ đồ kết nối

### Arduino UNO

```
L298N:
  ENA -> Pin 5 (PWM)
  IN1 -> Pin 6
  IN2 -> Pin 7
  IN3 -> Pin 8
  IN4 -> Pin 9
  ENB -> Pin 10 (PWM)

Cảm biến line:
  L_S -> A0 (Trái)
  C_S -> A1 (Giữa)
  R_S -> A2 (Phải)

HC-SR04:
  TRIG -> Pin 11
  ECHO -> Pin 12

ESP8266:
  RX -> Pin 4 (TX Arduino)
  TX -> Pin 3 (RX Arduino)
```

## 🚀 Cài đặt

1. **Nạp code cho Arduino UNO**: `mainUNO.ino`
2. **Nạp code cho ESP8266**: `esp8266/mainESp8266.ino`

## 📱 Sử dụng

1. Bật nguồn xe
2. Kết nối WiFi: `Car Arduino` / Password: `12345678`
3. Truy cập: `http://192.168.4.1`
4. Chọn chế độ AUTO hoặc MANUAL

## ⚙️ Cấu hình PID

Điều chỉnh trong `mainUNO.ino`:

```cpp
float Kp = 28;    // Proportional
float Ki = 0.05;  // Integral
float Kd = 4;     // Derivative
```

## 📁 Cấu trúc

```
XeArduino/
├── mainUNO.ino          # Code Arduino UNO
├── esp8266/
│   └── mainESp8266.ino  # Code ESP8266
└── README.md
```

## 📄 License

MIT License
