#include <NewPing.h>
#include <Servo.h>
#include <AFMotor.h>

#define RIGHT A2
#define LEFT A3
#define TRIGGER_PIN A1
#define ECHO_PIN A0
#define MAX_DISTANCE 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

AF_DCMotor Motor1(1, MOTOR12_1KHZ);
AF_DCMotor Motor2(2, MOTOR12_1KHZ);
AF_DCMotor Motor3(3, MOTOR34_1KHZ);
AF_DCMotor Motor4(4, MOTOR34_1KHZ);

Servo myservo;
int currentServoAngle = 90;

bool objectDetected = false;
unsigned long lastSeenTime = 0;

void smoothMoveServo(int targetAngle) {
  if (targetAngle == currentServoAngle) return;
  int step = (targetAngle > currentServoAngle) ? 1 : -1;
  for (int pos = currentServoAngle; pos != targetAngle; pos += step) {
    myservo.write(pos);
    delay(10);
  }
  myservo.write(targetAngle);
  currentServoAngle = targetAngle;
}

unsigned int getStableDistance() {
  unsigned int sum = 0;
  int count = 0;
  for (int i = 0; i < 5; i++) {
    unsigned int d = sonar.ping_cm();
    if (d > 0) {
      sum += d;
      count++;
    }
    delay(10);
  }
  return (count > 0) ? (sum / count) : MAX_DISTANCE;
}

int readStableIR(int pin) {
  int total = 0;
  for (int i = 0; i < 3; i++) {
    total += digitalRead(pin);
    delay(5);
  }
  return (total >= 2) ? 1 : 0;
}

void setup() {
  Serial.begin(9600);
  myservo.attach(10);
  smoothMoveServo(90);
  pinMode(RIGHT, INPUT);
  pinMode(LEFT, INPUT);
}

void moveForward(int speed) {
  Motor1.setSpeed(speed); Motor1.run(FORWARD);
  Motor2.setSpeed(speed); Motor2.run(FORWARD);
  Motor3.setSpeed(speed); Motor3.run(FORWARD);
  Motor4.setSpeed(speed); Motor4.run(FORWARD);
}

void stopMotors() {
  Motor1.run(RELEASE);
  Motor2.run(RELEASE);
  Motor3.run(RELEASE);
  Motor4.run(RELEASE);
}

void turnLeft() {
  Motor1.setSpeed(130); Motor1.run(BACKWARD);
  Motor3.setSpeed(130); Motor3.run(BACKWARD);
  Motor2.setSpeed(150); Motor2.run(FORWARD);
  Motor4.setSpeed(150); Motor4.run(FORWARD);
}

void turnRight() {
  Motor2.setSpeed(130); Motor2.run(BACKWARD);
  Motor4.setSpeed(130); Motor4.run(BACKWARD);
  Motor1.setSpeed(150); Motor1.run(FORWARD);
  Motor3.setSpeed(150); Motor3.run(FORWARD);
}

void loop() {
  int leftIR = readStableIR(LEFT);
  int rightIR = readStableIR(RIGHT);
  int distance = getStableDistance();

  Serial.print("Distance: "); Serial.println(distance);
  Serial.print("Left IR: "); Serial.print(leftIR);
  Serial.print(" | Right IR: "); Serial.println(rightIR);

  if (leftIR == 1 && rightIR == 0) {
    turnRight(); 
    smoothMoveServo(60);  // Turn servo left
          // Rotate robot left
    objectDetected = true;
  } else if (rightIR == 1 && leftIR == 0) {
    smoothMoveServo(120); // Turn servo right
 
    turnLeft();          // Rotate robot right
    objectDetected = true;
  } else if (distance > 10 && distance < 100 ||(rightIR == 0 && leftIR == 0)){
    smoothMoveServo(90);  // Face front
    moveForward(150);     // Move fast
    objectDetected = true;
  } else if (distance >= 30 && distance < 100) {
    smoothMoveServo(90);  // Face front
    moveForward(100);     // Move slow
    objectDetected = true;
  } else {
    stopMotors();
    smoothMoveServo(90);  // Reset to center
    objectDetected = false;
  }

  delay(50); // small delay for sensor stability
}
