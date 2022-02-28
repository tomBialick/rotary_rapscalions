#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int ROTARY_DIAL = A3;
const int CALL_PIN = 7;
const int RING_PIN = 8;
const int VSS_ONE = 9;
const int VSS_TWO = 10;
const int PIEZO_PIN = 6;
const int HIGH_THRESH = 400; 
const int LOW_THRESH = 25; 
const int PULSE_TIME = 50; //ms
const int PAUSE_TIME = 50; //ms
const int HOOK_CHANGE_THRESH = 250; //ms
const int RING_PERIOD = 50; //ms
const int RING_LENGTH = 1000; //ms

enum PhoneStatus {
  null_state,
  on_hook,
  off_hook,
  dialing,
  calling,
  ringing
};

enum RingStatus {
  left,
  right,
  pause
};

PhoneStatus phone; 
RingStatus ring;
int pulseBuffer;
unsigned long currTime;
unsigned long lastTimeHigh;
unsigned long lastTimeLow;
unsigned long lastTimePulse;
unsigned long ringTime;
unsigned long pauseTime;
bool pulseCountedFlag;
char* phoneNumber;
int digitCounter;

void setup() {
  Serial.begin(9600);
  
  phone = null_state;
  ring = left;
  pulseBuffer = 0;
  currTime = 0;
  lastTimeHigh = 0;
  lastTimeLow = 0;
  ringTime = 0;
  pauseTime = 0;
  pulseCountedFlag = false;

  lcd.begin(16, 2);

  pinMode(CALL_PIN, INPUT);
  pinMode(RING_PIN, INPUT);
  pinMode(VSS_ONE, OUTPUT);
  pinMode(VSS_TWO, OUTPUT);
  pinMode(PIEZO_PIN, OUTPUT);

  phoneNumber = (char*) malloc(17 * sizeof(char));
  digitCounter = 0;
}

void nullState(int analRead) {
  pulseBuffer = 0;
  pulseCountedFlag = false;
  if (analRead <= LOW_THRESH && (lastTimeHigh + HOOK_CHANGE_THRESH) <= currTime) {
    phone = on_hook;
    lastTimeLow = millis();
  }
  else if (analRead >= HIGH_THRESH && (lastTimeLow  + HOOK_CHANGE_THRESH) <= currTime) {
    phone = off_hook;
    lastTimeHigh = millis();
  }
}

void onHook(int analRead, int digiReadRing) {
  pulseBuffer = 0;
  if (digitCounter != 0) {
    free(phoneNumber);
    digitCounter = 0;
    phoneNumber = (char*) malloc(17 * sizeof(char));
  }
  pulseCountedFlag = false;
  if (analRead >= HIGH_THRESH && (lastTimeLow  + HOOK_CHANGE_THRESH) <= currTime) {
    phone = off_hook;
    lastTimeHigh = millis();
  }
  else if (digiReadRing == HIGH) {
    phone = ringing;
    ring = left;
    pauseTime = millis();
    if (analRead >= HIGH_THRESH) {
      lastTimeHigh = millis();
    }
  }
  else if(analRead <= LOW_THRESH) {
    lastTimeLow = millis();
  }
  lcd.clear();
}

void offHook(int analRead, int digiReadCall) {
  pulseCountedFlag = false;
  if (analRead <= LOW_THRESH && (lastTimeHigh + HOOK_CHANGE_THRESH) <= currTime) {
    phone = on_hook;
    lastTimeLow = millis();
  }
  else if (analRead <= LOW_THRESH && !pulseCountedFlag && (lastTimeHigh + PULSE_TIME) < currTime ) {
    phone = dialing;
    pulseBuffer++;
    lastTimeLow = millis();
    pulseCountedFlag = true;
  }
  else if (digiReadCall == HIGH) {
    phone = calling;
    if (analRead >= HIGH_THRESH) {
      lastTimeHigh = millis();
    }
  }
  else if (analRead >= HIGH_THRESH) {
    lastTimeHigh = millis();
  }
}

void dialingState(int analRead) {
  if (analRead <= LOW_THRESH && (lastTimeHigh + HOOK_CHANGE_THRESH) <= currTime) {
    phone = on_hook;
    lastTimeLow = millis();
    pulseBuffer = 0;
  }
  else if (analRead <= LOW_THRESH && !pulseCountedFlag && (lastTimeHigh + PULSE_TIME) < currTime ) {
    phone = dialing;
    pulseBuffer++;
    lastTimeLow = millis();
    pulseCountedFlag = true;
  }
  else if (analRead >= HIGH_THRESH && (lastTimeLow + PAUSE_TIME) < currTime ) {
    phone = off_hook;
    lastTimeHigh = millis();
    if (digitCounter < 16) {
      printDialed(pulseBuffer);
      phoneNumber[digitCounter] = pulseBuffer == 10 ? '0': pulseBuffer + '0';
      digitCounter++;
    }
    pulseBuffer = 0;
  }
  if(analRead <= LOW_THRESH) {
    lastTimeLow = millis();
  }
  else if (analRead >= HIGH_THRESH) {
    lastTimeHigh = millis();
    pulseCountedFlag = false;
  }
}

//will need to pass the phone number pointer
void callingState(int analRead) {
  if (digitCounter != 0) {
    phoneNumber[digitCounter] = '\0';
    lcd.setCursor(0, 1);
    lcd.print("Calling...");
    Serial.println(phoneNumber);
    digitCounter = 0;
    free(phoneNumber);
  }
  if (analRead >= HIGH_THRESH) {
    lastTimeHigh = millis();
  }
  else if (analRead <= LOW_THRESH && (lastTimeHigh + HOOK_CHANGE_THRESH) <= currTime) {
    phone = on_hook;
    lastTimeLow = millis();
  }
}

void ringingState(int analRead) {
  if (analRead >= HIGH_THRESH && (lastTimeLow  + HOOK_CHANGE_THRESH) <= currTime) {
    phone = off_hook;
    lastTimeHigh = millis();
    digitalWrite(VSS_ONE, LOW);
    digitalWrite(VSS_TWO, LOW);
    noTone(PIEZO_PIN);
  }
  Serial.println(ring);
  switch (ring) {
    case pause:
      if ((currTime - pauseTime) <= RING_LENGTH) {
        digitalWrite(VSS_ONE, LOW);
        digitalWrite(VSS_TWO, LOW);
        noTone(PIEZO_PIN);
      }
      else {
        pauseTime = millis();
        ring = left;
      }
      break;
    case left:
    case right:
      if ((currTime - pauseTime) <= RING_LENGTH) {
        if ((currTime - ringTime) >= RING_PERIOD/2) {
          ringTime = currTime;
          tone(PIEZO_PIN, 250);
          switch(ring) {
            case left:
              digitalWrite(VSS_ONE, HIGH);
              digitalWrite(VSS_TWO, LOW);
              ring = right;
              break;
            case right:
              digitalWrite(VSS_ONE, LOW);
              digitalWrite(VSS_TWO, HIGH);
              ring = left;
              break;
          }
        }
      }
      else {
        pauseTime = millis();
        ring = pause;
      }
      break;
  }
  
  if(analRead <= LOW_THRESH) {
    lastTimeLow = millis();
  }
}

void printDialed(int lastDialed) {
  int digit = lastDialed == 10 ? 0: lastDialed;
  lcd.print(digit);
}

void loop() {
  int analRead = analogRead(ROTARY_DIAL);
  int digiReadCall = digitalRead(CALL_PIN);
  int digiReadRing = digitalRead(RING_PIN);
  currTime = millis();
  
  switch (phone) {
    case null_state:
      nullState(analRead);
      break;
    case on_hook:
      onHook(analRead, digiReadRing);
      break;
    case off_hook:
      offHook(analRead, digiReadCall);
      break;
    case dialing:
      dialingState(analRead);
      break;
    case calling:
      callingState(analRead);
      break;
    case ringing:
      ringingState(analRead);
      break;
  }
}
