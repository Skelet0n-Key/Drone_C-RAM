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

volatile unsigned long timer1_millis = 0;
unsigned long millis_without_coords = 0;

const double X_CURVE_COEFFICIENT = 2;
const int X_FREQ_MAX = 2000;
const int X_FREQ_MIN = 31;
const int X_ACCEL_LIMIT = 50;
const double Y_CURVE_COEFFICIENT = 4;
const int Y_FREQ_MAX = 2000;
const int Y_FREQ_MIN = 31;
const int Y_ACCEL_LIMIT = 50;

const int X_DEADZONE = 20;  // true deadzone is times two
const int Y_DEADZONE = 20;

// Function prototypes
void stopTimer0();
void startTimer0();
void stopTimer2();
void startTimer2();
void setxFreq(int xFreq);
void setyFreq(int yFreq);
int xfrequency_calculator(int coord, int curr_freq, int center);
int yfrequency_calculator(int coord, int curr_freq, int center);
void set_dir(char axis, int center, int coord);

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

void loop() {
  unsigned long curr_millis = timer1_millis_get();

  if (curr_millis - millis_without_coords >= 500) {
    stopTimer0();
    stopTimer2();
  }

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
      millis_without_coords = curr_millis;
    } else coords += c;
  }
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
      startTimer0();
      digitalWrite(xDirPin, LOW);
    } else if (coord - center < -1*X_DEADZONE) {
      startTimer0();
      digitalWrite(xDirPin, HIGH);
    } else {
      stopTimer0();
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
