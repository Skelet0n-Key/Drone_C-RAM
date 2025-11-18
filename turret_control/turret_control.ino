#include <Arduino.h>

String coords = "";
int x = 0;
int y = 0;
int centerX = 320;
int centerY = 240;
int xFreq = 100;
int yFreq = 100;

int xPulsePin = 9;   // Timer1 OC1A
int xDirPin = 8;
int yPulsePin = 11;  // Timer2 OC2A
int yDirPin = 10;

const double X_CURVE_COEFFICIENT = 5;
const int X_FREQ_MAX = 2000;
const int X_FREQ_MIN = 100;
const int X_ACCEL_LIMIT = 30;
const double Y_CURVE_COEFFICIENT = 3;
const int Y_FREQ_MAX = 1000;
const int Y_FREQ_MIN = 31;
const int Y_ACCEL_LIMIT = 20;

const int X_DEADZONE = 20;  // true deadzone is times two
const int Y_DEADZONE = 20;

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

void setup() {
  // Timer1 - prescaler 256
  TCCR1A = (1 << COM1A0);
  TCCR1B = (1 << WGM12) | (1 << CS12);
  OCR1A = (F_CPU / (2 * 256 * 1000)) - 1;

  // Timer2 - prescaler 1024
  TCCR2A = (1 << COM2A0) | (1 << WGM21);
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
  OCR2A = (uint8_t)((F_CPU / (2UL * 1024UL * 1000UL)) - 1);

  pinMode(xPulsePin, OUTPUT);
  pinMode(xDirPin, OUTPUT);
  pinMode(yPulsePin, OUTPUT);
  pinMode(yDirPin, OUTPUT);

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
  OCR1A = (F_CPU / (2UL * 256UL * (unsigned long)xFreq)) - 1;
}

void setyFreq(int yFreq) {
  OCR2A = (F_CPU / (2UL * 1024UL * (unsigned long)yFreq)) - 1;
}

int xfrequency_calculator(int coord, int curr_freq, int center) {
  int target_freq = constrain(abs(coord-center) * X_CURVE_COEFFICIENT, X_FREQ_MIN, X_FREQ_MAX);

  Serial.println(coord);
  Serial.println(target_freq);
  Serial.println(curr_freq);

  if (curr_freq < target_freq) {
    return constrain(curr_freq + X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
  } else if (curr_freq > target_freq) {
    return constrain(curr_freq - X_ACCEL_LIMIT, X_FREQ_MIN, X_FREQ_MAX);
  }
  return curr_freq;
}

int yfrequency_calculator(int coord, int curr_freq, int center) {
  int target_freq = constrain(abs(coord-center) * Y_CURVE_COEFFICIENT, Y_FREQ_MIN, Y_FREQ_MAX);

  if (curr_freq < target_freq) {
    return constrain(curr_freq + Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
  } else if (curr_freq > target_freq) {
    return constrain(curr_freq - Y_ACCEL_LIMIT, Y_FREQ_MIN, Y_FREQ_MAX);
  }
  return curr_freq;
}

void set_dir(char axis, int center, int coord) {
  if (axis == 'x') {
    if (coord - center > X_DEADZONE) {
      startTimer1();
      digitalWrite(xDirPin, LOW);
    } else if (coord - center < -1*X_DEADZONE) {
      startTimer1();
      digitalWrite(xDirPin, HIGH);
    } else {
      stopTimer1();
    }
  } else if (axis == 'y') {
    if (coord - center > Y_DEADZONE) {
      startTimer2();
      digitalWrite(yDirPin, LOW);
    } else if (coord - center < -1*Y_DEADZONE) {
      startTimer2();
      digitalWrite(yDirPin, HIGH);
    } else {
      stopTimer2();
    }
  }
}
