// INCLUDE LIBRARIES FOR DISPLAY
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>


#include <TimeLib.h>
#include <TimeAlarms.h>

/*************************
** BEGIN ROTARY ENCODER **
*************************/
#define CLK  3 // digital 3 can be used as interrupt on UNO
#define DT  5
#define SW  2 // digital 2 can be used as interrupt on UNO

// STATE MANAGEMENT FOR ROTARTY ENCODER
int currentStateCLK;
int currentValue=0;
int lastValue=0;
String currentDir=""; 
unsigned long lastButtonInterrupt = 0;
unsigned long lastRun=0;
/***********************
** END ROTARY ENCODER **
************************/

/***********************
****  BEGIN DISPLAY ****
************************/
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
//for atmega328p 3.3v
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10
#define MAX_DEVICES 4
MD_Parola matrixDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String timeStr = "-- : --";
/***********************
*****  END DISPLAY *****
************************/

/****************************
**** BEGIN CLOCK & TIMER ****
****************************/
String hourStr;
String minStr;
bool hasHourBeenSet = false;
bool hasMinBeenSet = false;
bool hasTimeBeenConfirmed = false;
bool hasLightOnBeenSet = false;
bool displayChange = true;
bool hasLightOffBeenSet = false;
int tempHour = 0;
int tempMin = 0;
String clock;
/****************************
***** END CLOCK & TIMER *****
****************************/

void setup() {
  Serial.begin(9600);
  // Setup interrupts and pins for rotary encoder
  // IF LOW ENCODER IS TURNING
  attachInterrupt(digitalPinToInterrupt(CLK), handleEncoder, LOW);
  // IF FALLINB BUTTON WAS PROBABLY* PRESSED
  attachInterrupt(digitalPinToInterrupt(SW), handleButton,  FALLING);
  pinMode(SW, INPUT_PULLUP);
  pinMode(DT, INPUT);
  currentStateCLK = digitalRead(CLK);

  // initialize display
  matrixDisplay.begin();
  matrixDisplay.setIntensity(1);

  // welcome message
  matrixDisplay.setTextAlignment(PA_CENTER);
  matrixDisplay.print("Light :)");
  delay(2000);

  //set time to arbitrary number just to initialize library
  setTime(0,0,0,0,0,0); 

  updateDisplay(timeStr);
}

void updateDisplay (String toDisplay) {
  matrixDisplay.displayClear();
  matrixDisplay.setTextAlignment(PA_CENTER);
  matrixDisplay.print(toDisplay);
}

/*
    The initial time setup works in three steps
    1. set time: @hasTimeBeenConfirmed
    2. set LIGHTS ON time @hasLightOnBeenSet
    3. set LIGHTS OFF time @hasLightOffBeenSet
    When initially powered on the user must set the time, and both alarms for lights
    on and lights off. Then the program is running
*/
void handleButton() {
  // digitalRead(SW) == LOW even the interrupt to FALLING
  // sometimes the signal drops while the encoder is rotating but it's NOT a button press
  if(digitalRead(SW) == LOW) {
    if(millis() - lastButtonInterrupt > 30) {
      Serial.println("Button has really been pressed");
      if(!hasHourBeenSet) {
        hasHourBeenSet = true;
        return;
      }

      if(!hasMinBeenSet) {
        hasMinBeenSet = true;
        return;
      }
      
      // Hours and mins has been confirmed, 
      // save values and reset to move onto next step
      if(!hasTimeBeenConfirmed) {
        setTime(tempHour,tempMin,0,1,1,11);         
        hasTimeBeenConfirmed = true;
        resetTimeVals();
        formatTime(tempHour, tempMin);
        displayChange = true;
        return;
      } 

      // set on time
      if(!hasLightOnBeenSet) {
        Alarm.alarmRepeat(tempHour, tempMin, 0, turnOnLights);
        hasLightOnBeenSet = true;
        
        resetTimeVals();
        formatTime(tempHour, tempMin);
        displayChange = true;
        return;
      }

      // set off time
      if(!hasLightOffBeenSet) {
        hasTimeBeenConfirmed = true;
        Alarm.alarmRepeat(tempHour, tempMin, 0, turnOffLights);
        hasLightOffBeenSet = true;
        displayChange = true;
        updateDisplay("Ready");
        return;
      }

      return;
    }
  }
  //this is useful for debouncing, otherwise multiple values can be read from on 
  // user interrupt
  lastButtonInterrupt = millis();
}

void turnOnLights() {
  Serial.println("turning on lights");
  //TODO: map to analog OUTPUT pin to trigger on lights transistor   
}

void turnOffLights() {
  Serial.println("turning off lights");
  //TODO: map to analog OUTPUT pin to trigger off lights transistor
}

void resetTimeVals() {
  tempHour = 0;
  tempMin = 0;
  hasHourBeenSet = false;
  hasMinBeenSet = false;
}

void formatTime(int hr, int min) {
  if(hr < 10) {
    hourStr = '0' + String(hr);
  } else {
    hourStr = String(hr);
  }
  
  if(min < 10) {
    minStr = '0' + String(min);
  } else {
    minStr = String(min);
  }

  timeStr =  hourStr + ':' + minStr;

  lastRun=millis();
  lastValue = currentValue;
  displayChange = true;
}

/*
    The encoder is attached to
    Name | number | mapping 
    CLK  | 3:     | set as interrupt and INPUT
    DT   | 5:     | INPUT only
    SW   | 2:     | set as interrupt and INPUT
    This method only deals with:
    1. determining rotation direction
    2 .mapping those values to time 
*/
void handleEncoder() {
  if (millis()-lastRun>5){
    // Read the current state of CLK 
    currentStateCLK = digitalRead(CLK);

    // If the DT state is different than the CLK then encoder is rotating 
    // Counter clockwise so decrement
    if(digitalRead(DT) != currentStateCLK) {
      currentValue --;
    } else {
      // Encoder is rotating Clockwise so increment
      currentValue ++;
    }
  }

  // this check is easier to handle than tuning the 
  // debounced readings from the encoder
  if(currentValue != lastValue) {
    if(!hasHourBeenSet  ) {
      // 1. set hours first
      if(currentValue > lastValue) {
        tempHour ++;
      } else {
        tempHour --;
      } 
    } else if (!hasMinBeenSet && hasHourBeenSet) {
        // 2. set  minutes
      if(currentValue > lastValue) {
        tempMin ++;
      } else {
        tempMin --;
      } 
    }
    formatTime(tempHour, tempMin);
  }
}



void loop() {  
  if(displayChange) {
      updateDisplay(timeStr);
      displayChange = false;
  }

  if(hasLightOffBeenSet) {
    Serial.print("time | ");
    Serial.println(hour());
    Serial.println(minute());
    formatTime(hour(), minute());
    updateDisplay(timeStr);
    // This has to be called, the alarms are triggered in the delay
    Alarm.delay(60000); // wait one minute between clock display
  }
}