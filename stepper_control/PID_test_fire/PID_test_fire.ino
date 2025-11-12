#include <Arduino.h>

String coords = "";
int x = 0;
int y = 0;
int centerX = 320;
int centerY = 240;
int xFreq = 122;
int yFreq = 122;

int xPulsePin = 9;   // Timer1 OC1A
int xDirPin = 8;
int yPulsePin = 11;  // Timer2 OC2A
int yDirPin = 10;

// ULN2003 stepper pins
const int STEP_IN1 = 2;
const int STEP_IN2 = 3;
const int STEP_IN3 = 4;
const int STEP_IN4 = 5;

const int STEPS_PER_REV = 2048;
const int STEPS_PER_EIGHTH = STEPS_PER_REV / 2;

// Half-step sequence
const int sequence[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};

// Stepper state
struct StepperState {
  int currentStep = 0;
  int stepsRemaining = 0;
  int direction = 1;
  unsigned long lastStepTime = 0;
  int stepDelay = 1; // ms
  bool active = false;
  bool stateFired = false; // has fire been triggered
} stepper;

// Function prototypes
void stopTimer1();
void startTimer1();
void stopTimer2();
void startTimer2();
void setxFreq(int xFreq);
void setyFreq(int yFreq);
int xfrequency_calculator(int coord, int curr_freq, int center);
int yfrequency_calculator(int coord, int curr_freq, int center);
void set_dir(char axis, int center, int coord);

// Stepper functions
void updateStepper();
void driveStepper(bool fire);
void releaseCoils();

void setup() {
  // Timer1 - prescaler 8
  TCCR1A = (1 << COM1A0);
  TCCR1B = (1 << WGM12) | (1 << CS11);
  OCR1A = (F_CPU / (2 * 8 * 1000)) - 1;

  // Timer2 - prescaler 64
  TCCR2A = (1 << COM2A0) | (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = (uint8_t)((F_CPU / (2UL * 64UL * 1000UL)) - 1);

  pinMode(xPulsePin, OUTPUT);
  pinMode(xDirPin, OUTPUT);
  pinMode(yPulsePin, OUTPUT);
  pinMode(yDirPin, OUTPUT);

  // Stepper pins
  for(int i = STEP_IN1; i <= STEP_IN4; i++) pinMode(i, OUTPUT);

  Serial.begin(115200);

  stopTimer1();
  stopTimer2();
}

void loop() {
  // Serial input parsing
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      int commaIndex = coords.indexOf(',');
      x = coords.substring(0, commaIndex).toInt();
      y = coords.substring(commaIndex + 1).toInt();

      set_dir('x', centerX, x);
      set_dir('y', centerY, y);

      xFreq = xfrequency_calculator(x, xFreq, centerX);
      yFreq = yfrequency_calculator(y, yFreq, centerY);

      setxFreq(xFreq);
      setyFreq(yFreq);

      coords = "";
    } else coords += c;
  }

  // Check if turret is in dead zone
  bool xDead = abs(x - centerX) <= 20;
  bool yDead = abs(y - centerY) <= 20;

  if (xDead && yDead) {
    // Fire when in dead zone
    driveStepper(true);
  } else {
    // Reverse to original position when leaving dead zone
    driveStepper(false);
  }

  // Non-blocking stepper update
  updateStepper();
}

// ---------------- Stepper Functions ----------------
void updateStepper() {
  if (!stepper.active) return;

  unsigned long now = millis();
  if (now - stepper.lastStepTime >= stepper.stepDelay) {
    stepper.currentStep = (stepper.currentStep + stepper.direction + 8) % 8;
    for (int i = 0; i < 4; i++) {
      digitalWrite(STEP_IN1 + i, sequence[stepper.currentStep][i]);
    }
    stepper.lastStepTime = now;
    stepper.stepsRemaining--;

    if (stepper.stepsRemaining <= 0) {
      stepper.active = false;
      releaseCoils();
    }
  }
}

void driveStepper(bool fire) {
  if (fire && !stepper.stateFired && !stepper.active) {
    stepper.active = true;
    stepper.direction = 1;
    stepper.stepsRemaining = STEPS_PER_EIGHTH;
    stepper.stateFired = true;
  } else if (!fire && stepper.stateFired && !stepper.active) {
    stepper.active = true;
    stepper.direction = -1;
    stepper.stepsRemaining = STEPS_PER_EIGHTH;
    stepper.stateFired = false;
  }
}

void releaseCoils() {
  for (int i = STEP_IN1; i <= STEP_IN4; i++) digitalWrite(i, LOW);
}

// timer and frequency functions
void stopTimer1() {
  TCCR1A &= ~(1 << COM1A0);
  digitalWrite(xPulsePin, LOW);
}

void startTimer1() {
  TCCR1A |= (1 << COM1A0);
}

void stopTimer2() {
  TCCR2A &= ~(1 << COM2A0);
  digitalWrite(yPulsePin, LOW);
}

void startTimer2() {
  TCCR2A |= (1 << COM2A0);
}

void setxFreq(int xFreq) {
  OCR1A = (F_CPU / (2UL * 8UL * (unsigned long)xFreq)) - 1;
}

void setyFreq(int yFreq) {
  OCR2A = (uint8_t)((F_CPU / (2UL * 64UL * (unsigned long)yFreq)) - 1);
}

int xfrequency_calculator(int coord, int curr_freq, int center) {
  if (curr_freq > 2*abs(coord-center)) {  // deceleration curve
    if ((int)((double)curr_freq*0.95 < 122)) return 122;
    return (int)((double)curr_freq*0.95);
  } else {
    if ((int)((double)curr_freq*1.05 > 1000)) return 1000;  // acceleration curve
    return (int)((double)curr_freq*1.05);
  }
}

in yfrequency_calculator(int coord, int curr_freq, int center) {
  if (curr_freq > 2*abs(coord-center)) {
    if ((int)((double)curr_freq*0.95 < 122)) return 122;
    return (int)((double)curr_freq*0.95);
  } else {
    if ((int)((double)curr_freq*1.05 > 1000)) return 1000;  // acceleration curve
    return (int)((double)curr_freq*1.05);
  }
}

void set_dir(char axis, int center, int coord) {
  if (axis == 'x') {
    if (coord - center > 20) {
      startTimer1();
      digitalWrite(xDirPin, LOW);
    } else if (coord - center < -20) {
      startTimer1();
      digitalWrite(xDirPin, HIGH);
    } else {
      stopTimer1();
    }
  } else if (axis == 'y') {
    if (coord - center > 20) {
      startTimer2();
      digitalWrite(yDirPin, LOW);
    } else if (coord - center < -20) {
      startTimer2();
      digitalWrite(yDirPin, HIGH);
    } else {
      stopTimer2();
    }
  }
}
