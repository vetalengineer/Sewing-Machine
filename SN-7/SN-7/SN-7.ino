#define DIR_PIN 9
#define STEP_PIN 8
#define UPDOWN_PIN 10
#define NDL_PIN 11
#define DRIVER_STEP_TIME 200

enum MODS {
  ZIGZAG  
};

#include "GyverStepper.h"
GStepper <STEPPER2WIRE> stepper(400, STEP_PIN, DIR_PIN);

byte speedtime = 200;
int totalPos = 0;
byte mode = 0; 
boolean dir = 1;
byte stepsByStitch = 0;
boolean changeFlag = true;

int stitches = 1; //Количество Стежков 
int scale = 40;   //Масштаб в ширину 
                   //SCALE SET:0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5);
  setZeroPos();
  delay(1000);
}
//*************************************************************************
void loop() {
    changeStitch();
    if (scale > 66) scale = 66;
    if (digitalRead(UPDOWN_PIN))  modes(mode, stitches, scale);
}

void logS(String(input)) {Serial.println(input);}

void modes(byte mode, byte stitches, byte scale) {
  switch (ZIGZAG) {
    case 0:
      if (((dir)?(-scale/2):(scale/2) != totalPos) && changeFlag){
        changeFlag = !changeFlag;
        moveByStep(scale/2, !dir);
        while (digitalRead(UPDOWN_PIN)) {}
      }
      stepsByStitch = round(scale/stitches);
      Serial.println(" stepsByStitch = " + String(stepsByStitch));
      for (int i = 0; i < stitches; i++) {
        while (!digitalRead(UPDOWN_PIN)) {}         // Ждем пока игла снова не поднимется вверх
        logS("M: dir = " + String(dir));      
        moveByStep(stepsByStitch, dir);        
        while (digitalRead(UPDOWN_PIN)) {}          // ждем пока датчик снова не покажет что игла пошла вни
      }
      dir = !dir;
      break;  
  }
}

void setZeroPos() {
   if (!digitalRead(UPDOWN_PIN)) {
    Serial.println("Plese, lift up needle!");
    while (!digitalRead(UPDOWN_PIN)) {}
  }
  boolean flag = digitalRead(NDL_PIN); 
  while(flag?(digitalRead(NDL_PIN)):(!digitalRead(NDL_PIN))) {    
    moveByStep(1, !digitalRead(NDL_PIN));
    delay(1);
  }
  moveByStep(17 + 30, 1); 
  posReset();
}

void moveByStep(int stepsF, boolean dirF) {
  totalPos = (dirF)?(totalPos + stepsF):(totalPos - stepsF);
  //Serial.println("--totalPos = " + String(totalPos));
  //Serial.println("M : scale = " + String(scale) + " stitches = " + String(stitches));
  digitalWrite(DIR_PIN, dirF);
  for (int i = 0; i < stepsF; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(speedtime);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(speedtime);
  }
}

void changeStitch(){
  int lastScale = scale;
  int lastTotalPos = totalPos;
  if (Serial.available() > 1){  // n2 - установить два стежка, s33 - установить масштаб 33
    char key = Serial.read();
    int val = Serial.parseInt();
    switch(key) {
      case 'n':
        if (val != 0) stitches = abs(val);
        break;
      case 's':
        scale = abs(val);
        break;
    }
    logS("scale = " + String(scale) + " stitches = " + String(stitches));
    if (!digitalRead(UPDOWN_PIN)) {
      logS("Plese, lift up needle!");
      while (!digitalRead(UPDOWN_PIN)) {}
    }
    if (scale !=0 && lastScale == 0) {                                               // В ОБЩЕМ НУЖНО ПРИ ИЗМЕНЕНИИ МАСШТАБА В 0 ИЛИ НАОБОРОТ УЧИТЫАТЬ ПРОШЛЫЙ МАСШТАБ
      moveByStep(scale/2, 0);
      dir = 1;
    } else if (scale == 0 && lastScale != 0) {
      moveByStep(abs(totalPos), !(totalPos >= 0));
    }else {
      moveByStep(abs(-scale/2 - totalPos), (-scale/2 > totalPos));
      dir = 1;
    }
    while (digitalRead(UPDOWN_PIN)) {} 
  } 
}

void posReset() {totalPos = 0;}
