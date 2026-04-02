#include "HX711.h"

#define HX_DT A0
#define HX_SCK A1
HX711 scale;

float calibration_factor = 100.5;

// ===== YOUR ORIGINAL CNC CODE =====
#define X_STEP_PIN 2
#define X_DIR_PIN 5
#define X_LIMIT_PIN 9

#define Y_STEP_PIN 3
#define Y_DIR_PIN 6
#define Y_LIMIT_PIN 10

#define EN_PIN 8

bool lastXState = false;
bool lastYState = false;

// ===== ADDITION =====
unsigned long lastCheck = 0;
bool allowRun = false;

// ===== STOP SIGNAL =====
#define STOP_SIGNAL_PIN 12

bool stopTriggered = false;
bool reverseMode = false;
float weight = 0;

// ===== RELAY =====
#define RELAY_PIN 13
bool relayTriggeredOnce = false;

int relayStage = 0;
unsigned long relayTimer = 0;
bool relayRunning = false;

void setup() {
  Serial.begin(9600);

  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_LIMIT_PIN, INPUT_PULLUP);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_LIMIT_PIN, INPUT_PULLUP);

  pinMode(EN_PIN, OUTPUT);

  pinMode(STOP_SIGNAL_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  digitalWrite(EN_PIN, LOW);

  digitalWrite(X_DIR_PIN, LOW);
  digitalWrite(Y_DIR_PIN, LOW);

  Serial.println("SYSTEM STARTED");
}

void loop() {

  // ===== LOAD CELL CHECK =====
  if (millis() - lastCheck >= 5000) {
    lastCheck = millis();

    weight = scale.get_units();
    Serial.println(weight);

    // 🔥 FIXED LOGIC
    if (weight > 200 && !stopTriggered) {
      allowRun = true;
      reverseMode = false;
    } else {
      allowRun = false;

      // 🔥 KEY: reverse whenever weight is gone
      if (weight < 200) {
        reverseMode = true;
      }
    }
  }

  // ===== STOP SIGNAL =====
  if (digitalRead(STOP_SIGNAL_PIN) == HIGH) {
    stopTriggered = true;
    allowRun = false;

    if (!relayTriggeredOnce) {
      relayRunning = true;
      relayStage = 1;
      relayTimer = millis();
      relayTriggeredOnce = true;

      digitalWrite(RELAY_PIN, HIGH);
    }
  }

  // ===== RELAY SEQUENCE =====
  if (relayRunning) {
    switch (relayStage) {

      case 1:
        if (millis() - relayTimer >= 1000) {
          digitalWrite(RELAY_PIN, LOW);
          relayStage = 2;
          relayTimer = millis();
        }
        break;

      case 2:
        if (millis() - relayTimer >= 1000) {
          digitalWrite(RELAY_PIN, HIGH);
          relayStage = 3;
          relayTimer = millis();
        }
        break;

      case 3:
        if (millis() - relayTimer >= 1000) {
          digitalWrite(RELAY_PIN, LOW);
          relayStage = 4;
          relayTimer = millis();
        }
        break;

      case 4:
        if (millis() - relayTimer >= 1000) {
          digitalWrite(RELAY_PIN, HIGH);
          relayStage = 5;
          relayTimer = millis();
        }
        break;

      case 5:
        if (millis() - relayTimer >= 3000) {
          digitalWrite(RELAY_PIN, LOW);
          relayStage = 6;
          relayTimer = millis();
        }
        break;

      case 6:
        if (millis() - relayTimer >= 3000) {
          digitalWrite(RELAY_PIN, HIGH);
          relayStage = 7;
          relayTimer = millis();
        }
        break;

      case 7:
        if (millis() - relayTimer >= 1000) {
          digitalWrite(RELAY_PIN, LOW);
          relayRunning = false;
        }
        break;
    }
  }

  // ===== LIMIT SWITCHES =====
  bool xPressed = digitalRead(X_LIMIT_PIN);
  bool yPressed = digitalRead(Y_LIMIT_PIN);

  if (xPressed != lastXState) lastXState = xPressed;
  if (yPressed != lastYState) lastYState = yPressed;

  // ===== DIRECTION =====
  if (reverseMode) {
    digitalWrite(X_DIR_PIN, HIGH);
    digitalWrite(Y_DIR_PIN, HIGH);
  } else {
    digitalWrite(X_DIR_PIN, LOW);
    digitalWrite(Y_DIR_PIN, LOW);
  }

  // ===== MOTOR =====
  if (allowRun || reverseMode) {

    if (!reverseMode) {

      // FORWARD → ignore limit
      digitalWrite(X_STEP_PIN, HIGH);
      delayMicroseconds(800);
      digitalWrite(X_STEP_PIN, LOW);

      digitalWrite(Y_STEP_PIN, HIGH);
      delayMicroseconds(800);
      digitalWrite(Y_STEP_PIN, LOW);

    } else {

      // REVERSE → use limit
      if (xPressed && yPressed) {
        reverseMode = false;
        stopTriggered = false;
        relayTriggeredOnce = false;
      } else {

        if (!xPressed) {
          digitalWrite(X_STEP_PIN, HIGH);
          delayMicroseconds(800);
          digitalWrite(X_STEP_PIN, LOW);
        }

        if (!yPressed) {
          digitalWrite(Y_STEP_PIN, HIGH);
          delayMicroseconds(800);
          digitalWrite(Y_STEP_PIN, LOW);
        }

      }
    }
  }

  delayMicroseconds(800);
}
