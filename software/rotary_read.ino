const int ROTARY_DIAL = A3;
const int HIGH_THRESH = 400; 
const int LOW_THRESH = 25; 
const int PULSE_TIME = 50; //ms
const int PAUSE_TIME = 50; //ms
const int HOOK_CHANGE_THRESH = 250; //ms

enum PhoneStatus {
  null_state,
  on_hook,
  off_hook,
  dialing
};

PhoneStatus phone; 
int pulseBuffer;
int currTime;
int lastTimeHigh;
int lastTimeLow;
int lastTimePulse;
bool pulseCountedFlag;
int lastDialed;

void setup() {
  Serial.begin(9600);
  
  phone = null_state;
  pulseBuffer = 0;
  currTime = 0;
  lastTimeHigh = 0;
  lastTimeLow = 0;
  pulseCountedFlag = false;
  lastDialed = -1;
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
  pulseCountedFlag = false;
  if (analRead >= HIGH_THRESH && (lastTimeLow  + HOOK_CHANGE_THRESH) <= currTime) {
    phone = off_hook;
    lastTimeHigh = millis();
  }
  else if(analRead <= LOW_THRESH) {
    lastTimeLow = millis();
  }
}

void offHook(int analRead) {
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
    lastDialed = pulseBuffer;
    Serial.println(lastDialed); //call when ready
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

void loop() {
  int analRead = analogRead(ROTARY_DIAL);
  currTime = millis();

  switch(phone) {
    case null_state:
      nullState(analRead);
      break;
    case on_hook:
      onHook(analRead);
      break;
    case off_hook:
      offHook(analRead);
      break;
    case dialing:
      dialingState(analRead);
      break;
  }
}
