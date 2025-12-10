/*
 Y tb6600 setting 4 microstep (on off off)
 X tb6600 setting 2b microstep(off on on)
 both amp setting 2.5 (off on on)
*/

#include <Arduino.h>

String coords = "";
int x = 0;
int y = 0;
int centerX = 320;
int centerY = 240;
int xFreq = 100;
int yFreq = 100;

int xPulsePin = 6;   // Timer0 OCR0A
int xDirPin = 8;
int yPulsePin = 11;  // Timer2 OCR2A
int yDirPin = 10;

// y-limit switches
const int Y_DOWN_LIMIT_PIN = 2;
const int Y_UP_LIMIT_PIN = 3;

volatile bool yLimitLock = false;
volatile bool yUpLimit = false;
volatile bool yDownLimit = false;
volatile bool yRecover = false;
volatile bool scan = false;

// ULN2003 stepper pins
const int STEP_IN1 = 4;
const int STEP_IN2 = 5;
const int STEP_IN3 = 12;
const int STEP_IN4 = 13;

int pinlist[] = {4, 5, 12, 13};

const int STEPS_PER_REV = 2048;
const int STEPS_PER_EIGHTH = STEPS_PER_REV / 2;

// Half-step sequence for trigger stepper
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


// Timeout vars
volatile unsigned long timer1_millis = 0;
unsigned long millis_without_coords = 0;

// Turret behavior params
const double X_CURVE_COEFFICIENT = 2;
const int X_FREQ_MAX = 2000;
const int X_FREQ_MIN = 31;
const int X_ACCEL_LIMIT = 70;
const double Y_CURVE_COEFFICIENT = 4;
const int Y_FREQ_MAX = 2000;
const int Y_FREQ_MIN = 31;
const int Y_ACCEL_LIMIT = 50;

const int X_DEADZONE = 15;  // true deadzone is times two
const int Y_DEADZONE = 15;

const int X_FIRE_DEADZONE = 30;
const int Y_FIRE_DEADZONE = 30;

// Function prototypes for turret control
void stopTimer0();
void startTimer0();
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
  // Timer0 - prescaler 1024, 1kHz output on Pin 6 (X-axis)
  TCCR0A = (1 << COM0A0) | (1 << WGM01);  // Toggle mode, CTC mode
  TCCR0B = (1 << CS02) | (1 << CS00);     // Prescaler 1024
  OCR0A = (uint8_t)((F_CPU / (2UL * 1024UL * 1000UL)) - 1);

  // Timer1 setup for 1ms interrupts
  TCCR1A = 0;  // Normal mode
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // CTC mode, prescaler 64
  TCNT1 = 0;
  OCR1A = 249;  // 16MHz / 64 / 250 = 1000 Hz (1ms)
  TIMSK1 |= (1 << OCIE1A);  // Enable compare interrupt
  sei();  // Enable global interrupts
  
  // Timer2 - prescaler 1024, 1kHz output on Pin 11 (Y-axis)
  TCCR2A = (1 << COM2A0) | (1 << WGM21);  // Toggle mode, CTC mode
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  // Prescaler 1024
  OCR2A = (uint8_t)((F_CPU / (2UL * 1024UL * 1000UL)) - 1);
  
  pinMode(xPulsePin, OUTPUT);
  pinMode(xDirPin, OUTPUT);
  pinMode(yPulsePin, OUTPUT);
  pinMode(yDirPin, OUTPUT);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  pinMode(Y_UP_LIMIT_PIN, INPUT_PULLUP);
  pinMode(Y_DOWN_LIMIT_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(Y_UP_LIMIT_PIN), yUpLimitISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(Y_DOWN_LIMIT_PIN), yDownLimitISR, FALLING);
  
  Serial.begin(115200);
  
  stopTimer0();  // Changed from stopTimer1()
  stopTimer2();
}

ISR(TIMER1_COMPA_vect) {
  timer1_millis++;
}

unsigned long timer1_millis_get() {
  unsigned long m;
  noInterrupts();
  m = timer1_millis;
  interrupts();
  return m;
}

void yUpLimitISR() {
  yRecover = true;
  yUpLimit = true;
  yDownLimit = false;
}

void yDownLimitISR() {
  yRecover = true;
  yUpLimit = false;
  yDownLimit = true;
}



// ===============================================================
// ===============================================================


void loop() {
  // Recover logic
  // if (yRecover) {
  //   stopTimer0();
  //   stopTimer2();

  //   if (yUpLimit) {
  //     digitalWrite(yDirPin, LOW);
  //     setyFreq(150);
  //     startTimer2();
  //   }
  //   if (yDownLimit) {
  //     digitalWrite(yDirPin, HIGH);
  //     setyFreq(250);
  //     startTimer2();
  //   }

  //   unsigned long yRecoverMillis = timer1_millis_get();
  //   while (timer1_millis_get() - yRecoverMillis <= 2000) {
  //     true;
  //   }
  //   stopTimer2();
  //   yRecover = false;
  //   yUpLimit = false;
  //   yDownLimit = false;
  //   scan = true;
  // }

  unsigned long curr_millis = timer1_millis_get();

  // Timeout logic
  if ((curr_millis - millis_without_coords >= 21) && (scan == false)) {
    stopTimer0();
    stopTimer2();
    // digitalWrite(yDirPin, LOW);
    // setyFreq(150);
    // startTimer2();
  }

  // if (scan == true) {
  //   digitalWrite(xDirPin, HIGH);
  //   setxFreq(50);
  //   startTimer0();
  // }

  // Serial input parsing
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      int commaIndex = coords.indexOf(',');
      x = coords.substring(0, commaIndex).toInt();
      y = coords.substring(commaIndex + 1).toInt();

      xFreq = x_controller(x, centerX, xFreq);
      yFreq = y_controller(y, centerY, yFreq);

      setxFreq(xFreq);
      setyFreq(yFreq);

      coords = "";
      millis_without_coords = curr_millis;
      scan = false;
    } else coords += c;
  }
  
  bool xDead = abs(x - centerX) <= X_FIRE_DEADZONE;
  bool yDead = abs(y - centerY) <= Y_FIRE_DEADZONE;

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

// timer and frequency functions
void stopTimer0() {
  TCCR0A &= ~(1 << COM0A0);  // Changed from TCCR1A and COM1A0
  digitalWrite(xPulsePin, LOW);
}

void startTimer0() {
  TCCR0A |= (1 << COM0A0);
}

void stopTimer2() {
  TCCR2A &= ~(1 << COM2A0);
  digitalWrite(yPulsePin, LOW);
}

void startTimer2() {
  TCCR2A |= (1 << COM2A0);
}

void setxFreq(int xFreq) {
  OCR0A = (F_CPU / (2UL * 1024UL * (unsigned long)xFreq)) - 1;  // Changed OCR1A to OCR0A, and 256 to 1024
}

void setyFreq(int yFreq) {
  OCR2A = (F_CPU / (2UL * 1024UL * (unsigned long)yFreq)) - 1;
}

int x_controller(int coord, int center, int curr_freq) {
  int distance = coord - center;
  int target_freq = constrain(abs(distance) * X_CURVE_COEFFICIENT, X_FREQ_MIN, X_FREQ_MAX);

 // ====== TRAVELLING POSITIVE ======

  if (digitalRead(xDirPin) == LOW) {
    if (distance > X_DEADZONE) {  // normal behavior: accel/decel based on target freq, we're already travelling in the right direction
      if (curr_freq < target_freq) {
        startTimer0();
        digitalWrite(xDirPin, LOW);
        return constrain(curr_freq + X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
      } else if (curr_freq > target_freq) {
        startTimer0();
        digitalWrite(xDirPin, LOW);
        return constrain(curr_freq - X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
      }
      startTimer0();
      digitalWrite(xDirPin, LOW);
      return curr_freq;
    } else if (distance < -1*X_DEADZONE) {  // dir is positive and we are travelling in the wrong direction; turn around
      if (curr_freq == X_FREQ_MIN) {  // end case: we've decelerated as much as we can, now we can turn around
        digitalWrite(xDirPin, HIGH);
        startTimer0();
        return curr_freq;
      }
      startTimer0();
      digitalWrite(xDirPin, LOW);
      return constrain(curr_freq - X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);  // decel
    } else {  // We're in the deadzone: halt
      stopTimer0();
      return X_FREQ_MIN;
    }

   // ====== TRAVELLING NEGATIVE ======

  } else {
    if (distance > X_DEADZONE) {  // we need to turn around if this is true
      if (curr_freq == X_FREQ_MIN) {
        startTimer0();
        digitalWrite(xDirPin, LOW);
        return curr_freq;
      }
      startTimer0();
      digitalWrite(xDirPin, HIGH);
      return constrain(curr_freq - X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);  // decel
    } else if (distance < -1*X_DEADZONE) {  // normal behavior, accel/decel based on target frequency
      if (curr_freq < target_freq) {
        startTimer0();
        digitalWrite(xDirPin, HIGH);
        return constrain(curr_freq + X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
      } else if (curr_freq > target_freq) {
        startTimer0();
        digitalWrite(xDirPin, HIGH);
        return constrain(curr_freq - X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
      }
      startTimer0();
      digitalWrite(xDirPin, HIGH);
      return curr_freq;
    } else {  // in deadzone, halt
      stopTimer0();
      return X_FREQ_MIN;
    } 
  }
}

int y_controller(int coord, int center, int curr_freq) {
  int distance = coord - center;
  int target_freq = constrain(abs(distance) * Y_CURVE_COEFFICIENT, Y_FREQ_MIN, Y_FREQ_MAX);

 // ====== TRAVELLING POSITIVE ======

  if (digitalRead(yDirPin) == LOW) {
    if (distance > Y_DEADZONE) {  // normal behavior: accel/decel based on target freq, we're already travelling in the right direction
      if (curr_freq < target_freq) {
        startTimer2();
        digitalWrite(yDirPin, LOW);
        return constrain(curr_freq + Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
      } else if (curr_freq > target_freq) {
        startTimer2();
        digitalWrite(yDirPin, LOW);
        return constrain(curr_freq - Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
      }
      startTimer2();
      digitalWrite(yDirPin, LOW);
      return curr_freq;
    } else if (distance < -1*Y_DEADZONE) {  // dir is positive and we are travelling in the wrong direction; turn around
      if (curr_freq == Y_FREQ_MIN) {  // end case: we've decelerated as much as we can, now we can turn around
        digitalWrite(yDirPin, HIGH);
        startTimer2();
        return curr_freq;
      }
      startTimer2();
      digitalWrite(yDirPin, LOW);
      return constrain(curr_freq - Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);  // decel
    } else {  // We're in the deadzone: halt
      stopTimer2();
      return Y_FREQ_MIN;
    }

   // ====== TRAVELLING NEGATIVE ======

  } else {
    if (distance > Y_DEADZONE) {  // we need to turn around if this is true
      if (curr_freq == Y_FREQ_MIN) {
        startTimer2();
        digitalWrite(yDirPin, LOW);
        return curr_freq;
      }
      startTimer2();
      digitalWrite(yDirPin, HIGH);
      return constrain(curr_freq - Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);  // decel
    } else if (distance < -1*Y_DEADZONE) {  // normal behavior, accel/decel based on target frequency
      if (curr_freq < target_freq) {
        startTimer2();
        digitalWrite(yDirPin, HIGH);
        return constrain(curr_freq + Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
      } else if (curr_freq > target_freq) {
        startTimer2();
        digitalWrite(yDirPin, HIGH);
        return constrain(curr_freq - Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
      }
      startTimer2();
      digitalWrite(yDirPin, HIGH);
      return curr_freq;
    } else {  // in deadzone, halt
      stopTimer2();
      return Y_FREQ_MIN;
    } 
  }
}

// Trigger stepper functions
void updateStepper() {
  if (!stepper.active) return;

  unsigned long now = timer1_millis_get();
  if (now - stepper.lastStepTime >= stepper.stepDelay) {
    stepper.currentStep = (stepper.currentStep + stepper.direction + 8) % 8;
    for (int i = 0; i < 4; i++) {
      digitalWrite(pinlist[i], sequence[stepper.currentStep][i]);
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
