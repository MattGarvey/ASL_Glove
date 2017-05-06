#include <LiquidCrystal.h>
#include <SD.h>

int thumb = A0; //analog pin 0
int pointer = A1;
int middle = A2;
int ring = A3;
int pinky = A4;
LiquidCrystal lcd(12, 10, 5, 4, 3, 2);

void setup(){
  Serial.begin(9600);
  
  lcd.begin(16, 2);
}

void loop(){
  Serial.print("Thumb = ");
  Serial.print(testVal(thumb));
  Serial.println();
  //printlcd(thumb);

  Serial.print("Pointer = ");
  Serial.print(testVal(pointer));
  Serial.println();

  Serial.print("Middle = ");
  Serial.print(testVal(middle));
  Serial.println();

  Serial.print("Ring = ");
  Serial.print(testVal(ring));
  Serial.println();

  Serial.print("Pinky = ");
  Serial.print(testVal(pinky));
  Serial.println();


}

int testVal(int flexSensorPin){
  int flexSensorReading = analogRead(flexSensorPin); 
  
  return map(flexSensorReading, 370, 739, 0, 100);
}

int rawVal(int flexSensorPin){
    return analogRead(flexSensorPin); 
}

void printlcd(int finger){
  lcd.clear();
  lcd.print("Raw value: ");
  lcd.print(rawVal(finger));
  lcd.setCursor(0,1);
  lcd.print("100 test: ");
  lcd.print(testVal(finger));
  delay(500);
}




