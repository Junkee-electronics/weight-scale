#include "HX711.h"
#include "LiquidCrystal_I2C.h"

HX711 scale;
LiquidCrystal_I2C lcd(0x27,16,2);

const byte percpin = 10;
const byte countpin = 9;
const byte checkpin = 8;
const byte mpin = 7;
const byte unitspin = 6;
const byte tarepin = 5;
const byte zeropin = 4;

String unitPrint;

int i;
int j;

long starttime;
long piececount;
long lbweight = 0;
long refreshtime = millis();
long buttontime = millis();

float weight;
float desire;
float weightPrint;
float old = 0;
float zeroofs;
float saved = 0;

bool unit = false;
bool perc = false;
bool count = false;
bool check = false;
bool m = false;
bool units = false;
bool tareu = false;
bool zero = false;

bool percstate = false;
bool countstate = false;
bool hold = false;
bool previous = false;

void setup() {

  pinMode (percpin, INPUT_PULLUP);
  pinMode (countpin, INPUT_PULLUP);
  pinMode (checkpin, INPUT_PULLUP);
  pinMode (mpin, INPUT_PULLUP);
  pinMode (unitspin, INPUT_PULLUP);
  pinMode (tarepin, INPUT_PULLUP);
  pinMode (zeropin, INPUT_PULLUP);

  lcd.begin();
  lcd.cursor();
  lcd.print("initialising");
  Serial.begin(9600);
  scale.begin(2,3);
  scale.set_scale(224.0);
  scale.tare();
  zeroofs = scale.read_average(200);
  delay(200);
  weight = (scale.get_units(100),4);
  lcd.clear();
}

void loop(){
  perc = !debounce(percpin);
  count = !debounce(countpin);
  check = !debounce(checkpin);
  m = !debounce(mpin);
  units = !debounce(unitspin);
  tareu = !debounce(tarepin);
  zero = !debounce(zeropin);

  //tares the scale - sets the current weight as relative zero
  if (tareu==true)scale.tare();
 
  //zeros the scale, as to show you the "absolute" weight, relative to inicialisation weight
  else if(zero==true)scale.set_offset(zeroofs);
 
  //saves current weight, to be checked anytime later by pressing check
  else if(m==true)saved=weight;

  //changes units between g/kg|Oz|Lb, doesn't do anything while count mode is active
  else if(units==true & (millis()-buttontime)>500){
    unit = !unit;
    buttontime = millis();
  }
  
  //shows you the weight saved by M+
  else if(check==true){
    if ((millis()-refreshtime)>500){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("saved");
    lcd.setCursor(0,1);
    lcd.print(saved);
    lcd.setCursor(8,1);
    lcd.print(unitPrint);
    refreshtime = millis();
    }
  }

  //enables you to measure how many pieces are on the scale after you set weight of 1 piece
  else if(count==true){
    countstate = !countstate;
    if (countstate){
        old = scale.get_units(100);
        lcd.clear();
        lcd.print("place one piece");
        buttontime = millis();
        while (((millis() - buttontime)<5000) & countstate){
          if (!debounce(countpin))countstate = !countstate;
         }
        weight = scale.get_units(100);
        old = weight - old;
      }
  }

  //after you set the desired weight, it shows you how many percent is on the scale. this value is calculated relative to the current tare weight.
  else if(perc==true){
    percstate = !percstate;
    buttontime = millis();
    while (((millis() - buttontime)<5000) & percstate){
      perc = !debounce(percpin);
      count = !debounce(countpin);
      check = !debounce(checkpin);
      m = !debounce(mpin);
        if (perc){
          buttontime = millis();
          if (hold) desire = desire + 1;
          else desire = desire + 0.1;
        }
        if (check){
          buttontime = millis();
          if (hold) desire = desire - 1;
          else desire = desire - 0.1;
        }
        if (count){
          buttontime = millis() - 5000;
        }
        if (m) percstate = !percstate;
      delay (40);
      if ((millis() - refreshtime)<300){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("set desire value");
        lcd.setCursor(0,1);
        lcd.print(desire);
        lcd.setCursor(8,1);
        lcd.print("grams");
        refreshtime = millis();
      }
    }
  }
  
weight = scale.get_units(50);
if ((millis()-refreshtime)>250 & !check){
    display();
    refreshtime = millis();
  }
}

bool debounce(int pin) {
  if (digitalRead(pin) == true){
    if (20<(millis() - starttime)<500){
      hold = false;
      previous = true;
      return true;
    }
    else if ((millis() - starttime)>=500){
      hold = true;
      previous = true;
      return true;
    }
    else if (previous != digitalRead(pin)) starttime = millis();
  }
  else {
    if ((millis() - starttime)>20){
      previous = false;
      return false;
    }
    else {
      previous = true;
      return true;
    }
  }
}

void display(){
    if(countstate){
      percstate = false;
      weightPrint = (int) ((weight / old)+0.5);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("count");
      if(piececount==1) unitPrint = "Pc";
      else unitPrint = "Pcs";
    }
    else if(percstate){
      weightPrint = (weight/desire)*100;
      unitPrint = "%";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("percent");
    }
    else{
      if (!unit){
        lbweight = 0;
          if (weight>=1000){
            weightPrint = weight/1000;
            unitPrint = "kg";
          }
          else{
            weightPrint = weight;
            unitPrint = "g";
          }
        }
        else{
            weightPrint = weight/28.34952;
            unitPrint = "oz";
            lbweight = (int) (weightPrint/16);
            weightPrint = weightPrint - (lbweight*16);
            }
          }
  if (!countstate & !percstate){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("weight");
  }
  if (lbweight > 0){
    lcd.setCursor(0,1);
    lcd.print(lbweight);
    lcd.setCursor(3,1);
    lcd.print("lb");
    lcd.setCursor(6,1);
    lcd.print(weightPrint);
    lcd.setCursor(12,1);
    lcd.print(unitPrint);
  }
  else
  {
    lcd.setCursor(0,1);
    lcd.print(weightPrint);
    lcd.setCursor(8,1);
    lcd.print(unitPrint);
  }
  return;
}
