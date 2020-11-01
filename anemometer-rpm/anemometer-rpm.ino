#define PHOTOSENSOR_PIN 1
#define LAST_EDGE_INDEX 1    // number of edges - 1
volatile unsigned long usecRevolveTimeStart;
volatile unsigned long usecRevolveTimeEnd;
volatile int edgeCount;
volatile bool interruptValid;

unsigned long msecAvgInterval = 2000;
unsigned long msecAvgIntervalStart;
unsigned long msecSubIntervalStart;
float lastAngularVelocity;
float integratedAngularVelocity;
float avgAngularVelocity;

bool toggleLed;
bool writeToSerial;

enum ReadState {WAIT, READING, DONE} bufReadState;
char inBuffer[64];
int const inBufLen = 63;
int bufPos;

void photoInterruptOn() {
  
  digitalWrite(LED_BUILTIN, toggleLed);
  toggleLed = !toggleLed;
  
  if (!interruptValid) {
    return;
  }

  if (edgeCount == 0) {
    usecRevolveTimeStart = micros();
  } else if (edgeCount == LAST_EDGE_INDEX) {
    usecRevolveTimeEnd = micros();
    interruptValid = false;
  }
  ++edgeCount;
}

void resetInterval() {
  msecAvgIntervalStart = millis();
  msecSubIntervalStart = msecAvgIntervalStart;
  lastAngularVelocity = 0.;
  integratedAngularVelocity = 0.;
  avgAngularVelocity = 0.;

  edgeCount = 0;
  usecRevolveTimeStart = 0;
  usecRevolveTimeEnd = 0;
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PHOTOSENSOR_PIN, INPUT);  // analog

  resetInterval();
  interruptValid = true;

  attachInterrupt(PHOTOSENSOR_PIN, photoInterruptOn, RISING);

  writeToSerial = false;
  bufReadState = WAIT;
  bufPos = 0;
  Serial.begin(9600);
  
}


void loop() {
  // read the input on analog pin 0:
  // int sensorValue = analogRead(PHOTOSENSOR_PIN);

  if (!interruptValid) {
    if (usecRevolveTimeEnd > usecRevolveTimeStart) {
      auto const usecElapsed = usecRevolveTimeEnd- usecRevolveTimeStart;
    
      float const angularVelocity = 360e6 / float(usecElapsed);
      auto const msecNow = millis();
      float const Tsub = float(msecNow - msecSubIntervalStart)/1000.;
      // net angle
      integratedAngularVelocity += 0.5*(angularVelocity + lastAngularVelocity)*Tsub;

      //Serial.println(usecElapsed);

      msecSubIntervalStart = msecNow;
      lastAngularVelocity = angularVelocity;

      if (msecNow >= msecAvgIntervalStart + msecAvgInterval) {
        float const Tint = float(msecNow - msecAvgIntervalStart)/1000.; // interval time in s
        avgAngularVelocity = integratedAngularVelocity/Tint;

        if (writeToSerial) {
          Serial.print("{\"vtheta\":");
          Serial.print(avgAngularVelocity);
          Serial.println("}");
        }
        msecAvgIntervalStart = msecNow;
        integratedAngularVelocity = 0.;
      }
    }


    edgeCount = 0;
    interruptValid = true;
  }

  int const data = Serial.read();
  if (data >= 0) {
    char const ch = char(data);
    if (ch == 'P') {         // pause sending data over serial
      writeToSerial = false;
    } else if (ch == 'R') {  // resume sending data over serial
      writeToSerial = true;
    } else if (ch == 'T') {
      bufReadState = READING;
    } else if (ch == ';') { // don't record
      bufReadState = DONE;
    }

    if (bufReadState == READING) {
      if (bufPos < inBufLen) {
        inBuffer[bufPos++] += ch;
      } else {  // error
        bufPos = 0;
        bufReadState = WAIT;
      }
    } else if (bufReadState == DONE) {
      inBuffer[bufPos] = '\0';
      if (inBuffer[0] = 'T') {
        int const msecNewInterval = atoi(&inBuffer[1]); // trim command char
        if (msecNewInterval > 500 && msecNewInterval < 60000) {
          interruptValid = false;
          msecAvgInterval = msecNewInterval;
          resetInterval();
          interruptValid = true;
        }
      }
      bufPos = 0;
      bufReadState = WAIT;
    }
  }

}
