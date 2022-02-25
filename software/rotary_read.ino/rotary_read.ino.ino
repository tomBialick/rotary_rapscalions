#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int ROTARY_DIAL = A3;
const int CALL_PIN = 7;
const int HIGH_THRESH = 400; 
const int LOW_THRESH = 25; 
const int PULSE_TIME = 50; //ms
const int PAUSE_TIME = 50; //ms
const int HOOK_CHANGE_THRESH = 250; //ms

enum PhoneStatus {
  null_state,
  on_hook,
  off_hook,
  dialing,
  calling
};

PhoneStatus phone; 
int pulseBuffer;
int currTime;
int lastTimeHigh;
int lastTimeLow;
int lastTimePulse;
bool pulseCountedFlag;
char* phoneNumber;
int digitCounter;

void setup() {
  Serial.begin(9600);
  
  phone = null_state;
  pulseBuffer = 0;
  currTime = 0;
  lastTimeHigh = 0;
  lastTimeLow = 0;
  pulseCountedFlag = false;

  lcd.begin(16, 2);

  pinMode(CALL_PIN, INPUT);

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

void onHook(int analRead) {
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
  else if(analRead <= LOW_THRESH) {
    lastTimeLow = millis();
  }
  lcd.clear();
}

void offHook(int analRead, int digiRead) {
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
  else if (digiRead == HIGH) {
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
  phoneNumber[digitCounter] = '\0';
  lcd.setCursor(0, 1);
  lcd.print("Calling...");
  if (analRead >= HIGH_THRESH) {
    lastTimeHigh = millis();
  }
  else if (analRead <= LOW_THRESH && (lastTimeHigh + HOOK_CHANGE_THRESH) <= currTime) {
    phone = on_hook;
    lastTimeLow = millis();
  }
  if (digitCounter != 0) {
    Serial.println(phoneNumber);
    digitCounter = 0;
    free(phoneNumber);
  }
}

void printDialed(int lastDialed) {
  int digit = lastDialed == 10 ? 0: lastDialed;
  lcd.print(digit);
}

void loop() {
  int analRead = analogRead(ROTARY_DIAL);
  int digiRead = digitalRead(CALL_PIN);
  currTime = millis();
  
  switch(phone) {
    case null_state:
      nullState(analRead);
      break;
    case on_hook:
      onHook(analRead);
      break;
    case off_hook:
      offHook(analRead, digiRead);
      break;
    case dialing:
      dialingState(analRead);
      break;
    case calling:
      callingState(analRead);
      break;
  }
}
