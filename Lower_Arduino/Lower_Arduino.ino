/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(10, 9, 5, 7, 3, 2);

File myFile;
int accx[20];
int accy[20];
int accz[20];
int gyrx[20];
int gyry[20];
int gyrz[20];

int READ_DELAY = 500;
int TH = 100;
int MTH = 1000;


int thumb;
int pointer;
int middle;
int ring;
int pinky;
int mvmt;
char letter;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  //  Serial.print("Initializing SD card...");
  //
  //  if (!SD.begin(4)) {
  //    Serial.println("initialization failed!");
  //    return;
  //  }
  //  Serial.println("initialization done.");
  SD.begin(4);
  pinMode(6, OUTPUT);
}

void loop() {
  start();
  grabRaw();
  detectChars();

}

void start() {
  while (analogRead(A5) < 1000) {
  }
}

void writeChar() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {

    myFile.print(thumb);
    myFile.print(",");
    myFile.print(pointer);
    myFile.print(",");
    myFile.print(middle);
    myFile.print(",");
    myFile.print(ring);
    myFile.print(",");
    myFile.println(pinky);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void detectChars() {
  // format on sd (no spaces, seperated by comas)

  // int,int,int,int,int,mvmt,char

  // ex: 123,456,789,321,654,0,a

  // (thumb),(pointer),(middle),(ring),(pinky),(movement),(letter)
  myFile = SD.open("test.txt", FILE_READ);
  //  myFile = SD.open("test.txt");
  if (myFile) {
    int difS = 10000;
    char letterS = ' ';
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      int t = myFile.parseInt();
      int p = myFile.parseInt();
      int m = myFile.parseInt();
      int r = myFile.parseInt();
      int y = myFile.parseInt();
      int mv = myFile.parseInt();
      char letter1 = myFile.read(); // unused, passes cursor over the , that seperates int and char
      char letter = myFile.read();
      if ((thumb > t - TH && thumb < t + TH) &&
          (pointer > p - TH && pointer < p + TH) &&
          (middle > m - TH && middle < m + TH) &&
          (ring > r - TH && ring < r + TH) &&
          (pinky > y - TH && pinky < y + TH)) {

        int thumbComp = thumb - t;
        int pointerComp = pointer - p;
        int middleComp = middle - m;
        int ringComp = ring - r;
        int pinkyComp = pinky - y;

        int dif = abs(thumbComp) + abs(pointerComp) + abs(middleComp) + abs(ringComp) + abs(pinkyComp);
        if (dif < difS) {
          difS = dif;
          letterS = letter;
        }
      }

    }

    //If the letter is 'd' or 'z', determine if there was motion
    if (letterS == 'd' || letterS == 'z') {
      int m;
      int mx = 0;
      int lx = 30000;

      //default the letter to 'd'
      letterS = 'd';

      //find the maximum acceleration value
      for (int i = 0; i < 15; i++) {
        m = abs(accx[i]);
        if (m > mx) {
          mx = m;
        }
        //find the minimum acceleration value
        else if (m < lx && m != 0) {
          lx = m;
        }
      }

      //If the motion is greater than the threshold, the letter is 'z'
      if ((mx - lx) > MTH) {
        letterS = 'z';
      }

    }

    //If the letter is 'i' or 'i', determine if there was motion
    else if (letterS == 'i' || letterS == 'j') {
      int m;
      int mx = 0;
      int lx = 30000;

      //default the letter to 'd'
      letterS = 'i';
      for (int i = 0; i < 15; i++) {
        m = abs(accx[i]);
        if (m > mx) {
          mx = m;
        }
        else if (m < lx && m != 0) {
          lx = m;
        }
      }
      //If motion is greater than threshold, letter is 'j'
      if (mx - lx > MTH) {
        letterS = 'j';
      }
    }


    lcd.clear();
    lcd.print(letterS);
    Serial.print(letterS);
    // Seeks back to the first entry
    myFile.seek(0);
    myFile.close();

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test DETECT.txt");
  }
  myFile.close();
}

void charDict() {

}



void grabRaw() {
  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  thumb = Serial.parseInt();
  pointer = Serial.parseInt();
  middle = Serial.parseInt();
  ring = Serial.parseInt();
  pinky = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int i = 0; i < 15; i++)
    accx[i] = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int j = 0; j < 15; j++)
    accy[j] = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int k = 0; k < 15; k++)
    accz[k] = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int l = 0; l < 15; l++)
    gyrx[l] = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int m = 0; m < 15; m++)
    gyry[m] = Serial.parseInt();

  digitalWrite(6, HIGH);
  delay(READ_DELAY);
  digitalWrite(6, LOW);
  delay(READ_DELAY);

  for (int n = 0; n < 15; n++)
    gyrz[n] = Serial.parseInt();

  Serial.println(thumb);
  Serial.println(pointer);
  Serial.println(middle);
  Serial.println(ring);
  Serial.println(pinky);

  for (int i = 0; i < 15; i++) {
    Serial.print(accx[i]);
    Serial.print(", ");
  }
  Serial.println("");
  for (int j = 0; j < 15; j++) {
    Serial.print(accy[j]);
    Serial.print(", ");
  }
  Serial.println("");
  for (int k = 0; k < 15; k++) {
    Serial.print(accz[k]);
    Serial.print(", ");
  }
  Serial.println("");
  for (int l = 0; l < 15; l++) {
    Serial.print(gyrx[l]);
    Serial.print(", ");
  }
  Serial.println("");
  for (int m = 0; m < 15; m++) {
    Serial.print(gyry[m]);
    Serial.print(", ");
  }
  Serial.println("");
  for (int n = 0; n < 15; n++) {
    Serial.print(gyrz[n]);
    Serial.print(", ");
  }
  Serial.println("");
  while (Serial.available())
    Serial.read();
}
