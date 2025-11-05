String coords = "";
int x = 0;
int y = 0;
int centerX = 320;
int centerY = 240;
int xFreq = 0;
int yFreq = 0;

int xPulsePin = 9;   // Timer1 OC1A
int xDirPin = 8;
int yPulsePin = 11;  // Timer2 OC2A
int yDirPin = 10;

void setup() {
  // Timer1 - prescaler 8
  TCCR1A = (1 << COM1A0);
  TCCR1B = (1 << WGM12) | (1 << CS11);
  OCR1A = (F_CPU / (2 * 8 * 1000)) - 1;
  
  // Timer2 - prescaler 64
  TCCR2A = (1 << COM2A0) | (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = (uint8_t)((F_CPU / (2UL * 64UL * 1000UL)) - 1); // you wouldn't believe how stupid this integer overflow fix is
  
  pinMode(xPulsePin, OUTPUT);
  pinMode(xDirPin, OUTPUT);  // High is counter-clockwise, left
  pinMode(yPulsePin, OUTPUT);
  pinMode(yDirPin, OUTPUT);   // High is pitch up

  Serial.begin(115200);
}

// Function prototypes
void stopTimer1();
void startTimer1();
void stopTimer2();
void startTimer2();
void setxFreq(int xFreq);
void setyFreq(int yFreq);
int frequency_calculator(int coord, int curr_freq, int center);
void set_dir(char axis, int center, int coord);

void loop() {
  while (Serial.available() > 0) {  // receiving coords
    char c = Serial.read();
    if (c == '\n') {  // if we received full coords
      // parse coords
      int commaIndex = coords.indexOf(',');
      x = coords.substring(0, commaIndex).toInt();
      y = coords.substring(commaIndex + 1).toInt();
      // change direction with parsed coords
      set_dir('x', centerX, x);
      set_dir('y', centerY, y);
      // change frequency with parsed coords
      xFreq = frequency_calculator(x, xFreq, centerX);
      yFreq = frequency_calculator(y, yFreq, centerY);
      setxFreq(xFreq);
      setyFreq(yFreq);
      // --------------------------------------
      Serial.println("---------------------------------");
      Serial.println("xCoord: " + String(x));
      Serial.println("xFreq: " + String(xFreq));
      Serial.println("yCoord: " + String(y));
      Serial.println("yFreq: " + String(yFreq));

      Serial.println("OCR1A = " + String((F_CPU / (2UL * 8UL * (unsigned long)xFreq)) - 1));
      Serial.println("OCR2A = " + String((uint8_t)((F_CPU / (2UL * 64UL * (unsigned long)yFreq)) - 1)));
      // --------------------------------------
      coords = "";  // clear for next message
    } else coords += c;  // continue to append chars until '\n'
  }
  // timer frequencies cannot not go lower than 122Hz or higher than 1MHz
  // Tim2 OCR2A caps out at 255

  // our turret will be restricted to steps of 10ms (100Hz, but we can't go lower than 122Hz) and 0.5ms (2kHz) 

  // will change frequency and dir with pid based on aforementioned coords
}


void stopTimer1() {
  TCCR1A &= ~(1 << COM1A0);  // Disconnect OC1A
  digitalWrite(xPulsePin, LOW);      // Set pin low
}

void startTimer1() {
  TCCR1A |= (1 << COM1A0);   // Reconnect OC1A to timer
}

void stopTimer2() {
  TCCR2A &= ~(1 << COM2A0);  // Disconnect OC2A
  digitalWrite(yPulsePin, LOW);     // Set pin low
}

void startTimer2() {
  TCCR2A |= (1 << COM2A0);   // Reconnect OC2A to timer
}

void setxFreq(int xFreq) {
  OCR1A = (F_CPU / (2UL * 8UL * (unsigned long)xFreq)) - 1;
}

void setyFreq(int yFreq) {
  OCR2A = (uint8_t)((F_CPU / (2UL * 64UL * (unsigned long)yFreq)) - 1);
}

int frequency_calculator(int coord, int curr_freq, int center) {
  // returns a freq as a function of how far away the target is and how fast it's approaching it
  // a coord of -300 and +300 should return the same freq, we don't care abt dir
  if (curr_freq > 10*abs(coord-center)) {  // if approaching target too fast, slow down.
    if (10*abs(coord-center) < 122) {
      return 122;
    }
    return 10*abs(coord-center);
  } else {  // if approaching target and still accelerating, continue to accelerate
    if (curr_freq*2 > 2000) {
      return 2000;
    }
    return curr_freq*2;
  }
  // all hardcoded values subject to change. Just an example for now.
}

void set_dir(char axis, int center, int coord) {
  // x and y are on separate timers, we need to be able to distinguish here.
  // not possible to put this logic in the frequency calculator
  if (axis == 'x') {
    if (coord - center > 10) {
      startTimer1();  // outside of deadzone, connect timer to pin
      digitalWrite(xDirPin, LOW);  // moving clockwise
    } else if (coord - center < -10) {
      startTimer1();  // outside of deadzone
      digitalWrite(xDirPin, HIGH);  // moving counter-clockwise
    } else {
      stopTimer1();  // inside of deadzone, disconnect timer
    }
  } else if (axis == 'y') {
    if (coord - center > 10) {
      startTimer2();  // outside of deadzone, connect timer to pin
      digitalWrite(yDirPin, LOW);  // moving clockwise
    } else if (coord - center < -10) {
      startTimer2();  // outside of deadzone
      digitalWrite(yDirPin, HIGH);  // moving counter-clockwise
    } else {
      stopTimer2();  // inside of deadzone, disconnect timer
    }
  }
}