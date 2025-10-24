#include <ESP32Servo.h>

Servo myServo;

// Use D13 (GPIO 13)
int servoPin = 13;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing servo on GPIO 13...");

  // Attach servo to pin 13
  myServo.attach(servoPin);

  Serial.println("Servo is ready!");
}

void loop() {
  // Sweep from 0° to 180°
  for (int angle = 0; angle <= 180; angle++) {
    myServo.write(angle);
    delay(10); // Adjust for speed
  }

  // Sweep back from 180° to 0°
  for (int angle = 180; angle >= 0; angle--) {
    myServo.write(angle);
    delay(10);
  }
}
