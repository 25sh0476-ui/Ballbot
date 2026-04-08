/*
  ============================================================
  BALLBOT — ESP32 #1 — MOTOR CONTROL
  ============================================================
  L298N WIRING:
  ENA -> GPIO 25    ENB -> GPIO 13
  IN1 -> GPIO 26    IN3 -> GPIO 14
  IN2 -> GPIO 27    IN4 -> GPIO 12
  ============================================================
  WHEEL BEHAVIOUR (both parallel):
  Forward     -> Left=FULL FWD,  Right=FULL FWD
  Backward    -> Left=FULL BWD,  Right=FULL BWD
  Left        -> Left=FULL BWD,  Right=FULL FWD  (spin in place)
  Right       -> Left=FULL FWD,  Right=FULL BWD  (spin in place)
  Front-Left  -> Left=SLOW FWD,  Right=FULL FWD  (diagonal)
  Front-Right -> Left=FULL FWD,  Right=SLOW FWD  (diagonal)
  Back-Left   -> Left=SLOW BWD,  Right=FULL BWD  (diagonal)
  Back-Right  -> Left=FULL BWD,  Right=SLOW BWD  (diagonal)
  ============================================================
*/

#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ---- PINS ----
#define ENA  25
#define IN1  26
#define IN2  27
#define IN3  14
#define IN4  12
#define ENB  13

// ---- TUNING (adjust these!) ----
#define MOTOR_SPEED       200   // full speed  (0-255)
#define MOTOR_SPEED_SLOW  100   // diagonal slow wheel (0-255)

WebServer server(80);

// ============================================================
// MOTOR HELPERS
// ============================================================

void motorSetup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();
}

// Left wheel
void leftForward(int spd)  { analogWrite(ENA, spd); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); }
void leftBackward(int spd) { analogWrite(ENA, spd); digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); }
void leftStop()            { analogWrite(ENA, 0);   digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); }

// Right wheel
void rightForward(int spd)  { analogWrite(ENB, spd); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void rightBackward(int spd) { analogWrite(ENB, spd); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
void rightStop()            { analogWrite(ENB, 0);   digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW); }

// ---- MOVEMENTS ----
void moveForward()    { leftForward(MOTOR_SPEED);       rightForward(MOTOR_SPEED); }
void moveBackward()   { leftBackward(MOTOR_SPEED);      rightBackward(MOTOR_SPEED); }
void turnLeft()       { leftBackward(MOTOR_SPEED);      rightForward(MOTOR_SPEED); }
void turnRight()      { leftForward(MOTOR_SPEED);       rightBackward(MOTOR_SPEED); }
void moveFrontLeft()  { leftForward(MOTOR_SPEED_SLOW);  rightForward(MOTOR_SPEED); }
void moveFrontRight() { leftForward(MOTOR_SPEED);       rightForward(MOTOR_SPEED_SLOW); }
void moveBackLeft()   { leftBackward(MOTOR_SPEED_SLOW); rightBackward(MOTOR_SPEED); }
void moveBackRight()  { leftBackward(MOTOR_SPEED);      rightBackward(MOTOR_SPEED_SLOW); }
void stopMotors()     { leftStop(); rightStop(); }

// ============================================================
// HTTP HANDLERS
// ============================================================

void handleForward()    { moveForward();    server.send(200, "text/plain", "OK"); }
void handleBackward()   { moveBackward();   server.send(200, "text/plain", "OK"); }
void handleLeft()       { turnLeft();       server.send(200, "text/plain", "OK"); }
void handleRight()      { turnRight();      server.send(200, "text/plain", "OK"); }
void handleFrontLeft()  { moveFrontLeft();  server.send(200, "text/plain", "OK"); }
void handleFrontRight() { moveFrontRight(); server.send(200, "text/plain", "OK"); }
void handleBackLeft()   { moveBackLeft();   server.send(200, "text/plain", "OK"); }
void handleBackRight()  { moveBackRight();  server.send(200, "text/plain", "OK"); }
void handleStop()       { stopMotors();     server.send(200, "text/plain", "OK"); }

void handleStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "ONLINE");
}

// ============================================================
// SETUP & LOOP
// ============================================================

void setup() {
  Serial.begin(115200);
  motorSetup();

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nMotor ESP32 IP: " + WiFi.localIP().toString());

  server.enableCORS(true);

  server.on("/forward",    handleForward);
  server.on("/backward",   handleBackward);
  server.on("/left",       handleLeft);
  server.on("/right",      handleRight);
  server.on("/frontleft",  handleFrontLeft);
  server.on("/frontright", handleFrontRight);
  server.on("/backleft",   handleBackLeft);
  server.on("/backright",  handleBackRight);
  server.on("/stop",       handleStop);
  server.on("/status",     handleStatus);
  server.begin();
  Serial.println("Motor server ready!");
}

void loop() {
  server.handleClient();
}
