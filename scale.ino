#include "HX711.h"
#include "LiquidCrystal_I2C.h"
#include "AceButton.h"

HX711 scale;
LiquidCrystal_I2C lcd(0x27,16,2);

const int percpin =2;
const int countpin =3;
const int checkpin =4;
const int mpin =5;
const int unitspin =6;
const int tarepin =7;
const int zeropin =8;

byte unit = 0;

String unitPrint;

int i;

long oldtime;
long curtime;
long piececount;

float weight;
float weightPrint;
float old = 0;
float zeroofs;
float saved = 0;

bool perc;
bool count;
bool countstate;
bool check;
bool m;
bool units;
bool tare;
bool zero;


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

void display() {
  weight = scale.get_units(20);
    if(countstate == true){
      round(weightPrint = weight / old);

      if(piececount==1) unitPrint = "Pc";
      else unitPrint = "Pcs";
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
  lcd.clear();
  lcd.print(weightPrint);
  lcd.print("  ");
  lcd.print(unitPrint);
}

void loop() {
  if (tare==true)scale.tare();    
    else if(zero==true)scale.set_offset(zeroofs);
    else if(m==true)saved=weight;
    else if(units==true)(unit++)%3;
    else {if(count==true){
      returnpress:
      if (countstate == true)countstate = !countstate;
        else {
          old = weight;
          lcd.clear();
          lcd.print("place one piece");
          for ( i = 0; i < 1000 & !count; i++)delay(1);
          if (i!=1000)goto returnpress;
          scale.tare();
          old = weight - old;
          countstate = !countstate;
        }
      }
    else if(check==true){
      lcd.print(saved);
      lcd.print(" ");
      lcd.print(unitPrint);
      }
    else if(perc==true){

    }
    else{
    display();
    delay(100);
    }
  }
}
