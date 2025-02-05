#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 62 // Analog servos run at ~50 Hz updates
#define STEP_DELAY 20 // Delay between steps in milliseconds

// Array to hold current pulse widths for each servo
int currentPulses[6] = {0, 0, 0, 0, 0, 0}; // Assuming 6 servos
int targetPulses[6] = {0, 0, 0, 0, 0, 0}; // Target pulses for each servo
bool moving[6] = {false, false, false, false, false, false}; // Moving state for each servo

void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);
  
  initializeServos();
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    int separatorIndex = command.indexOf(' ');
    
    if (command.equals("STOP")) {
      for (int i = 0; i < 6; i++) {
        moving[i] = false; // Stop all movements
      }
      Serial.println("Emergency Stop: All servos halted.");
    } 
    else if (separatorIndex != -1) {
      int servoNum = command.substring(0, separatorIndex).toInt();
      int angle = command.substring(separatorIndex + 1).toInt();

      if (servoNum >= 0 && servoNum <= 5 && angle >= 0 && angle <= 180) {
        targetPulses[servoNum] = angleToPulse(angle, servoNum);
        moving[servoNum] = true; // Mark this servo as moving
      } else {
        Serial.println("Error: Invalid servo number or angle.");
      }
    }
  }

  // Move all servos that are marked as moving
  for (int i = 0; i < 6; i++) {
    moveServoSmooth(i);
  }

  delay(STEP_DELAY); // Add a small delay to control update frequency
}

void moveServoSmooth(int servoNum) {
  if (moving[servoNum]) {
    if (currentPulses[servoNum] < targetPulses[servoNum]) {
      currentPulses[servoNum] += (currentPulses[servoNum] + STEP_DELAY < targetPulses[servoNum]) ? 1 : 0;
    } else if (currentPulses[servoNum] > targetPulses[servoNum]) {
      currentPulses[servoNum] -= (currentPulses[servoNum] - STEP_DELAY > targetPulses[servoNum]) ? 1 : 0;
    } else {
      moving[servoNum] = false; // Stop moving if reached target
    }
    pwm.setPWM(servoNum, 0, currentPulses[servoNum]); // Update PWM signal
  }
}

int angleToPulse(int angle, int servoNum) {
  int minPulse, maxPulse;

  switch (servoNum) {
    case 0: 
      minPulse = 110; 
      maxPulse = 570;
      break; 
    case 1: minPulse = 100; maxPulse = 500; break; // Shoulder
    case 2: minPulse = 150; maxPulse = 500; break; // Elbow
    case 3: minPulse = 100; maxPulse = 500; break; // Wrist Y
    case 4: minPulse = 150; maxPulse = 400; break; // Wrist Rot
    case 5: minPulse = 170; maxPulse = 320; break; // Gripper
    default: minPulse = 150; maxPulse = 500; break;
  }

  angle = constrain(angle, 0, 180); // Ensure it's within bounds

  return map(angle, 0, 180, minPulse, maxPulse);
}


void initializeServos() {
  setServo(0, 170); // Base
  setServo(1, 90);  // Shoulder
  setServo(2, 180); // Elbow
  setServo(3, 80);  // Wrist Y
  setServo(4, 30);  // Wrist Rot
  setServo(5, 0);   // Gripper
}

// Helper function to set servo angle and update current pulse
void setServo(int servoNum, int angle) {
  int pulse = angleToPulse(angle, servoNum);
  pwm.setPWM(servoNum, 0, pulse);
  currentPulses[servoNum] = pulse; // Update the current pulse
}
