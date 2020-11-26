#include "HX711.h"
#include "LiquidCrystal_I2C.h"

HX711 scale;
LiquidCrystal_I2C lcd(0x27,16,2);

const byte percpin = 2;
const byte countpin = 3;
const byte checkpin = 4;
const byte mpin = 5;
const byte unitspin = 6;
const byte tarepin = 7;
const byte zeropin = 8;

byte unit = 0;

String unitPrint;

int i;
int j;
int k;

long starttime;
long piececount;

float weight;
float desire;
float weightPrint;
float old = 0;
float zeroofs;
float saved = 0;

bool perc;
bool count;
bool check;
bool m;
bool units;
bool tare;
bool zero;

bool percstate;
bool countstate;
bool hold;
bool previous;

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
  scale.begin(0,1);
  scale.set_scale(224.0);
  scale.tare();
  zeroofs = scale.read_average(20);
  delay(1000);
  weight = (scale.get_units(),3);
}

void display(){
  weight = scale.get_units(20);
    if(countstate == true){
      round(weightPrint = weight / old);

      if(piececount==1) unitPrint = "Pc";
      else unitPrint = "Pcs";
    }
    else if(percstate == true){

    }
    else{
      if (unit==0){
          if (weight>=1000){
            weightPrint = weight/1000;
            unitPrint = "kg";
          }
          else{
            weightPrint = weight;
            unitPrint = "g";
          }
        }
        else{if(unit==1){
            weightPrint = weight/28.34952;
            unitPrint = "oz";
          }
        else{
            weightPrint = weight/435.59237;
            unitPrint = "lb";
        }
      }
    }
  lcd.setCursor(1,0);
  lcd.print(weightPrint);
  lcd.print("  ");
  lcd.print(unitPrint);
  return;
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

void loop(){
for (j = 0; j < 200; j++){

  perc = debounce(percpin);
  count = debounce(countpin);
  check = debounce(checkpin);
  m = debounce(mpin);
  units = debounce(unitspin);
  tare = debounce(tarepin);
  zero = debounce(zeropin);

  //tares the scale - sets the current weight as relative zero
  if (tare==true)scale.tare();
  //zeros the scale, as to show you the "absolute" weight, relative to inicialisation weight
  else if(zero==true)scale.set_offset(zeroofs);
  //saves current weight, to be checked anytime later by pressing check
  else if(m==true)saved=weight;
  //changes units between g/kg|Oz|Lb, doesn't do anything while count mode is active
  else if(units==true)(unit++)%3;
  //enables you to measure how many pieces are on the scale after you set weight of 1 piece
  else if(count==true){
    returncount:
    if (countstate == true)countstate = !countstate;
      else {
        old = weight;
        lcd.clear();
        lcd.print("place one piece");
        for ( i = 0; i < 1000 & !count; i++){
          count = debounce(countpin);
          delay(2);
        }
        if (i!=1000)goto returncount;
        scale.tare();
        old = weight - old;
        countstate = !countstate;
      }
    }
  //shows you the weight saved by M+
  else if(check==true){
    lcd.print(saved);
    lcd.print(" ");
    lcd.print(unitPrint);
    }
  //after you set the desired weight, it shows you how many percent is on the scale. this value is calculated relative to the current tare weight.
  else if(perc==true){
    if (percstate == true)percstate = !percstate;
      else {
        old = weight;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("set desired weight");
        for ( i = 0; i < 200; i++){
          if ((perc = debounce(percpin)) == true){
            if (hold == false){
              desire+0.1;
            }
            else{
              desire++;
            }
          }
          else if ((count = debounce(countpin)) == true){
            i = 200;
          }
          else if ((check = debounce(checkpin)) == true){
            if (hold == false){
              desire-0.1;
            }
            else{
              desire--;
            }
          }
          lcd.setCursor(1,0);
          lcd.print(desire);
          delay(50);
        }
        percstate = !percstate;
      }
    }
  }
display();
}
