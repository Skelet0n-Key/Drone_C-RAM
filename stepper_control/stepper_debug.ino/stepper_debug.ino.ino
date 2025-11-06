const int step1out = 8;
const int dir1out  = 9;
const int step2out = 10;
const int dir2out  = 11;

const int stepsPerTest = 150; // total steps per move
const int startDelay = 10000;   // slowest delay (µs)
const int minDelay = 2000;     // fastest delay (µs)800 max
const int rampSteps = 15;    // how many steps to ramp up/down ~100 min

void setup() {
  pinMode(step1out, OUTPUT);
  pinMode(dir1out, OUTPUT);
  pinMode(step2out, OUTPUT);
  pinMode(dir2out, OUTPUT);

  // Initialize outputs
  digitalWrite(step1out, HIGH);
  digitalWrite(step2out, HIGH);
  digitalWrite(dir1out, HIGH);  // forward
  digitalWrite(dir2out, HIGH);

  delay(500);
}

void loop() {
  digitalWrite(dir1out, HIGH);
  digitalWrite(dir2out, HIGH);
  rampedMove(stepsPerTest);
  delay(1000);

  digitalWrite(dir1out, LOW);
  //digitalWrite(dir2out, LOW);
  rampedMove(stepsPerTest);
  delay(3000);
}

void rampedMove(int stepsTotal) {
  int delayTime = startDelay;
  int halfSteps = stepsTotal / 2;

  // --- ACCELERATION PHASE ---
  for (int i = 0; i < rampSteps; i++) {
    delayTime = map(i, 0, rampSteps, startDelay, minDelay);
    doStep(delayTime);
  }

  // --- CONSTANT SPEED PHASE ---
  for (int i = rampSteps; i < stepsTotal - rampSteps; i++) {
    doStep(minDelay);
  }

  // --- DECELERATION PHASE ---
  for (int i = 0; i < rampSteps; i++) {
    delayTime = map(i, 0, rampSteps, minDelay, startDelay);
    doStep(delayTime);
  }
}

void doStep(int delayTime) {
  digitalWrite(step1out, LOW);
  digitalWrite(step2out,LOW);
  delayMicroseconds(delayTime);
  digitalWrite(step1out, HIGH);
  digitalWrite(step2out, HIGH);
  delayMicroseconds(delayTime);
}
