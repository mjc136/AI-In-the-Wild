#include <WebUSB.h>
#include <Servo.h>

// WebUSB Setup
WebUSB WebUSBSerial(1, "webusb.github.io/arduino/demos/rgb");
#define Serial WebUSBSerial

Servo myservo;

const int SERVO_PIN = 9;
int pos = 0;
int colorIndex = 0;
int color[3];

void setup() {
  myservo.attach(SERVO_PIN);
  myservo.write(60);  // Initial servo position

  Serial.begin(9600);
  delay(1000); // Ensure WebUSB initializes properly

  Serial.write("Sketch begins.\r\n");
}

void loop() {
  if (Serial && Serial.available()) {
    color[colorIndex++] = Serial.read();

    if (colorIndex == 1) {
      Serial.print("Received: ");
      Serial.println(color[0]);

      if (color[0] == 1) {
        Serial.println("Cereal detected, EXTREME LEFT movement...");
        moveServo(90, 0);  // Move fully left (0°)
        delay(3000);       // Hold position for 3 sec
        moveServo(0, 90);  // Quickly return to center
      }
      else if (color[0] == 2) {
        Serial.println("Marshmallow detected, EXTREME RIGHT movement...");
        moveServo(90, 180);  // Move fully right (180°)
        delay(3000);         // Hold position for 3 sec
        moveServo(180, 90);  // Quickly return to center
      }


      while (Serial.available()) {
        Serial.read();  // Clear extra data
      }

      Serial.println("Buffer cleared.");
      colorIndex = 0;  // Reset for next input
    }
  }
  else {
    idleServoMotion();
  }
}

// Move the servo smoothly between two angles
void moveServo(int startAngle, int endAngle) {
  if (startAngle < endAngle) {
    for (pos = startAngle; pos <= endAngle; pos += 5) {
      myservo.write(pos);
      delay(10);
    }
  } else {
    for (pos = startAngle; pos >= endAngle; pos -= 5) {
      myservo.write(pos);
      delay(10);
    }
  }
}

// Idle movement when no objects detected
void idleServoMotion() {
  for (pos = 60; pos <= 90; pos += 1) {
    myservo.write(pos);
    delay(3);
  }
  for (pos = 90; pos >= 60; pos -= 1) {
    myservo.write(pos);
    delay(3);
  }
}
