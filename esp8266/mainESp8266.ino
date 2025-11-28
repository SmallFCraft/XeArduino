#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Cấu hình WiFi Access Point (Phat Wifi)
const char *ssid = "Car Arduino";
const char *password = "12345678";

ESP8266WebServer server(80);

// Trang HTML giao dien dieu khien
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Car Control</title>
  <style>
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background-color: #2c3e50; 
      color: white; 
      text-align: center; 
      margin: 0; 
      overflow: hidden; /* Prevent scroll */
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
    }
    h1 { margin: 10px 0; font-size: 1.5rem; }
    
    .mode-controls {
      display: flex;
      gap: 20px;
      margin-bottom: 20px;
      z-index: 10;
    }

    .btn {
      border: none;
      border-radius: 10px;
      padding: 15px 30px;
      font-size: 1rem;
      font-weight: bold;
      cursor: pointer;
      color: white;
      background-color: #34495e;
      box-shadow: 0 4px 6px rgba(0,0,0,0.3);
    }
    .btn.active { background-color: #3498db; border: 2px solid #fff; }

    #joystick-container {
      position: relative;
      width: 300px;
      height: 300px;
      background: rgba(255, 255, 255, 0.1);
      border-radius: 50%;
      border: 2px solid rgba(255, 255, 255, 0.2);
      touch-action: none; /* Important for touch */
    }

    #joystick-knob {
      position: absolute;
      width: 80px;
      height: 80px;
      background: #e74c3c;
      border-radius: 50%;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      box-shadow: 0 0 10px rgba(0,0,0,0.5);
      pointer-events: none; /* Let events pass to container */
    }
  </style>
</head>
<body>
  <h1>Car Control</h1>
  
  <div class="mode-controls">
    <button class="btn" onclick="setMode('A', this)">AUTO</button>
    <button class="btn" onclick="setMode('M', this)">MANUAL</button>
  </div>

  <div id="joystick-container">
    <div id="joystick-knob"></div>
  </div>

  <script>
    var container = document.getElementById('joystick-container');
    var knob = document.getElementById('joystick-knob');
    var rect = container.getBoundingClientRect();
    var center = { x: rect.width / 2, y: rect.height / 2 };
    var active = false;
    var lastCmd = 'S';

    function send(cmd) {
      if (cmd !== lastCmd) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "cmd?val=" + cmd, true);
        xhr.send();
        lastCmd = cmd;
      }
    }

    function setMode(mode, btn) {
      send(mode);
      document.querySelectorAll('.btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
    }

    function handleMove(clientX, clientY) {
      var x = clientX - rect.left - center.x;
      var y = clientY - rect.top - center.y;
      
      var distance = Math.sqrt(x*x + y*y);
      var maxDist = center.x - 40; // 40 is half knob size

      if (distance > maxDist) {
        var angle = Math.atan2(y, x);
        x = Math.cos(angle) * maxDist;
        y = Math.sin(angle) * maxDist;
      }

      knob.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;

      // Determine Direction
      // -45 to 45: Right
      // 45 to 135: Down (Screen Y is down) -> Backward
      // 135 to -135 (225): Left
      // -135 to -45: Up -> Forward
      
      var angleDeg = Math.atan2(y, x) * 180 / Math.PI;
      var cmd = 'S';

      if (distance > 20) { // Deadzone
        if (angleDeg > -45 && angleDeg <= 45) cmd = 'R';
        else if (angleDeg > 45 && angleDeg <= 135) cmd = 'B';
        else if (angleDeg > -135 && angleDeg <= -45) cmd = 'F';
        else cmd = 'L';
      } else {
        cmd = 'S';
      }
      send(cmd);
    }

    function reset() {
      active = false;
      knob.style.transform = `translate(-50%, -50%)`;
      send('S');
    }

    container.addEventListener('mousedown', e => { active = true; handleMove(e.clientX, e.clientY); });
    window.addEventListener('mousemove', e => { if(active) handleMove(e.clientX, e.clientY); });
    window.addEventListener('mouseup', reset);

    container.addEventListener('touchstart', e => { 
      active = true; 
      handleMove(e.touches[0].clientX, e.touches[0].clientY); 
      e.preventDefault(); 
    }, {passive: false});
    
    container.addEventListener('touchmove', e => { 
      if(active) handleMove(e.touches[0].clientX, e.touches[0].clientY); 
      e.preventDefault(); 
    }, {passive: false});
    
    container.addEventListener('touchend', reset);
  </script>
</body>
</html>
)=====";

void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleCmd() {
  String cmd = server.arg("val");
  Serial.print(cmd); // Gui lenh qua Serial cho Arduino
  server.send(200, "text/plain", cmd);
}

void setup() {
  Serial.begin(9600); // Giao tiep voi Arduino qua TX/RX
  
  // Setup Access Point
  WiFi.softAP(ssid, password);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);

  server.begin();
}

void loop() {
  server.handleClient();
}
