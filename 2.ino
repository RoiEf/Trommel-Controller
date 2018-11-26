#include <TM1637Display.h>

#define CLK 2
#define DIO 3

const int potPin = A0;  // connecting a pot to A0 in order to set starting value
int potValue;           // variable for initial pot state value
const int analogLow = 0;      // values comming from the analog read
const int analogHigh = 1023;  // values comming from the analog read

unsigned long currentMillis;
unsigned long previousMillis = 0;
const long interval = 500;  // set to half of wanted interval
int displayTime;        // the display time in seconds
int lastDisplayTime;    // last used display time
int blinkDots = 0;

int machineState = 0;   // 0 for stop. 1 for run
int machineStateError = 0;

const int NO_WATER_Pin = 13;  // used to connectto Error light
const int pb1Pin = 12;        // Push button 1 - start/puse + reset
const int pb2Pin = 11;        // Push button 2 - Add Time + reset
const int float1Pin = 10;
const int float2Pin = 9;
const int relayPin = 8;    // pin for connecting the relay to run stop machine
const int fillValvePin = 7;    // pin for connecting the relay to open water valve to fill up tank

int pb1State;             // save push buttons state from ISR
int pb2State;
int float1State;
int float2State;
int lastPb1State = LOW;
int lastPb2State = LOW;
int lastfloat1State = LOW;
int lastfloat2State = LOW;

unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime4 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

TM1637Display display(CLK, DIO); // create instance of the display

void setup() {
  pinMode(relayPin,OUTPUT);   // making sure machine dont run at the begining
  digitalWrite(relayPin,LOW);
  pinMode(fillValvePin,OUTPUT);   // making sure water fillup valve close at start of machine
  digitalWrite(fillValvePin,LOW);
  
  // Serial.begin(9600);   // Starting serial communication

  pinMode(potPin,INPUT);    // setting the pin for input
  potValue = map(analogRead(potPin),analogLow,analogHigh,10,30);

  pinMode(pb1Pin, INPUT);       // setting Push button pins for input.
  pinMode(pb2Pin, INPUT);
  pinMode(float1Pin, INPUT);    // setting pins connected floats for input
  pinMode(float2Pin, INPUT);
  pinMode(NO_WATER_Pin,OUTPUT); // Setting pin connected to the error light as output
   
  displayTime = potValue * 60;    // init the display time value
  lastDisplayTime = displayTime;

  display.setBrightness(0x0f);

//  while(!Serial);                     // Making sure serial communication work
  // Serial.println("Here we go! ");     // Printing welcome massege.
  // Serial.println("\n");
  // Serial.print("Start run time value: ");
  // Serial.print(displayTime/60);
  // Serial.print(" : ");
  // Serial.println(displayTime%60);
}

void loop() {

  int myTime = (displayTime/60)*100+displayTime%60;
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    if (blinkDots == 1) {
      display.showNumberDecEx(myTime, (0x80 >> 1), true,4,0);
      blinkDots = 0;
    } else {
    display.showNumberDec(myTime, true,4,0);
      blinkDots = 1;
      if (machineState == 1) {
        displayTime--;
        digitalWrite(relayPin,HIGH);
      }
    }

    previousMillis = currentMillis;
  }

  if (displayTime <= 0) {
    displayTime = lastDisplayTime;
    machineState = 0;
    digitalWrite(relayPin,LOW);
  }

//  int readFloat  = digitalRead(float1Pin);
//  if(readFloat == HIGH) {
//    digitalWrite(NO_WATER_Pin,HIGH);
//    machineStateError = 1;
//  } else {
//    digitalWrite(NO_WATER_Pin,LOW);
//    machineStateError = 0;
//  }

int reading1 = digitalRead(pb1Pin);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading1 != lastPb1State) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading1 != pb1State) {
      pb1State = reading1;

      if (pb1State == HIGH) {
        machineState = !machineState;
      }
    }
  }

  lastPb1State = reading1;


int reading2 = digitalRead(pb2Pin);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading2 != lastPb2State) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading2 != pb2State) {
      pb2State = reading2;

      // only toggle the LED if the new button state is HIGH
      if (pb2State == HIGH) {
        displayTime += 60;
        lastDisplayTime += 60;
      }
    }
  }

  lastPb2State = reading2;
  
int reading3 = digitalRead(float1Pin);
  // check to see if float changed
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading3 != lastfloat1State) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
  }

  if ((millis() - lastDebounceTime3) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading3 != float1State) {
      float1State = reading3;

      // only toggle the LED if the new button state is HIGH
      if (float1State == HIGH) {
        digitalWrite(NO_WATER_Pin,HIGH);
        machineStateError = machineState;
        machineState = 0;
        digitalWrite(fillValvePin,HIGH); // Open water fillup valve
      } else {
        digitalWrite(NO_WATER_Pin,LOW);
        machineState = machineStateError;
       }
    }
  }

  lastfloat1State = reading3;


  int readFloat2 = digitalRead(float2Pin);
  // check to see if float changed
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (readFloat2 != lastfloat2State) {
    // reset the debouncing timer
    lastDebounceTime4 = millis();
  }

  if ((millis() - lastDebounceTime4) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (readFloat2 != float2State) {
      float2State = readFloat2;

      // only toggle the LED if the new button state is HIGH
      if (float2State == HIGH) {
        digitalWrite(fillValvePin,LOW); // close fill up water valve
      }
    }
  }

  lastfloat2State = readFloat2;
}


