//Includes sending the values in groups of 15


//Nick Wolford: april 25
//Final revision for the code that will run on the sensor handling arduino for sign detection
//Utilizing the MPU6050 to read in values for the position and acceleration, must be tied to
//5 flex sensors will be measured as well
//running logs of the data will be kept and transmittted to the secondary arduino upon interrupt from the other arduino


//needed for the MPU6050/Serial
#include<Wire.h>
#include<stdlib.h>
#include<stdio.h>

//needed afor arduino interrupts to make the digitalPinToInterrupt() work
#define NOT_AN_INTERRUPT -1

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int k = 0;

//memory to hold the values read in from the sensors
int16_t* ACXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
int16_t* ACYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
int16_t* ACZhist = (int16_t *) malloc(sizeof(int16_t) * 20);

int16_t* GYXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
int16_t* GYYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
int16_t* GYZhist = (int16_t *) malloc(sizeof(int16_t) * 20);

int16_t* flexVals = (int16_t *) malloc(sizeof(int16_t) * 5);

int16_t* mpuOffsets = (int16_t *) malloc(sizeof(int16_t) * 6);

boolean dumpTrigger = 0;


void setup() {
  //general accelerometer setup
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);

  //need to set up the analog input of the flex sensors
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);

  pinMode(13, OUTPUT);
  pinMode(10, OUTPUT);

  delay(2000);

  //block to populate the memory with intial values and also let the values from the sensor settle out.
  for (int i = 19; i >= 0; i--) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers


    ACXhist[i] = Wire.read() << 8 | Wire.read();
    ACYhist[i] = Wire.read() << 8 | Wire.read();
    ACZhist[i] = Wire.read() << 8 | Wire.read();

    GYXhist[i] = Wire.read() << 8 | Wire.read();
    GYYhist[i] = Wire.read() << 8 | Wire.read();
    GYZhist[i] = Wire.read() << 8 | Wire.read();

    delay(10);
  }
  Wire.endTransmission(true);

  //populate flex values
  flexVals[0] = analogRead(A0);
  flexVals[1] = analogRead(A6);
  flexVals[2] = analogRead(A1);
  flexVals[3] = analogRead(A2);
  flexVals[4] = analogRead(A3);

  //setting offsets for the motion data, should bring them to 0-mean. This will make the computiations easier later, and only has to be done once here
  int total = 0;
  for (int i = 0; i < 20; i++) {
    total += (ACXhist[i] / 20);
  }
  mpuOffsets[0] = total;

  total = 0;
  for (int i = 0; i < 20; i++) {
    total += (ACYhist[i] / 20);
  }
  mpuOffsets[1] = total;

  total = 0;
  for (int i = 0; i < 20; i++) {
    total += (ACZhist[i] / 20);
  }
  mpuOffsets[2] = total;

  total = 0;
  for (int i = 0; i < 20; i++) {
    total += (GYXhist[i] / 20);
  }
  mpuOffsets[3] = total;

  total = 0;
  for (int i = 0; i < 20; i++) {
    total += (GYYhist[i] / 20);
  }
  mpuOffsets[4] = total;

  total = 0;
  for (int i = 0; i < 20; i++) {
    total += (GYZhist[i] / 20);
  }
  mpuOffsets[5] = total;

  //this is the setup for the interrupts from the other arduino.
  attachInterrupt(digitalPinToInterrupt(2), ISR0, RISING);

  Serial.end();

}

//simple state machine effectively, but too few states to bew worth writing out a big case => update structure.
void loop() {

  //if the interrupt has happened, then move on to the data transfer mode
  if (dumpTrigger) {
    k += 1; //k increases to send differnet groups of values
    dumpVals();
    dumpTrigger = false;
  }elseif( k != 0){
    
    //in normal operation just collect values from the sensors

  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);

  flexVals[0] = analogRead(A0);
  flexVals[1] = analogRead(A6);
  flexVals[2] = analogRead(A1);
  flexVals[3] = analogRead(A2);
  flexVals[4] = analogRead(A3);

  for (int i = 19; i >= 0; i--) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers

    ACXhist[i] = Wire.read() << 8 | Wire.read();
    ACXhist[i] = ACXhist[i] - mpuOffsets[0];
    ACYhist[i] = Wire.read() << 8 | Wire.read();
    ACYhist[i] = ACYhist[i] - mpuOffsets[1];
    ACZhist[i] = Wire.read() << 8 | Wire.read();
    ACZhist[i] = ACZhist[i] - mpuOffsets[2];
    GYXhist[i] = Wire.read() << 8 | Wire.read();
    GYXhist[i] = GYXhist[i] - mpuOffsets[3];
    GYYhist[i] = Wire.read() << 8 | Wire.read();
    GYYhist[i] = GYYhist[i] - mpuOffsets[4];
    GYZhist[i] = Wire.read() << 8 | Wire.read();
    GYZhist[i] = GYZhist[i] - mpuOffsets[5];
  }

    delay(75);

  }



  Wire.endTransmission(true);

}

//here is where the interrupt will send the data back to the other arduino upon interrupt
//need 7 total interrupts to send all of the data and revert back to the initial mode
void dumpVals() {

  digitalWrite(10, HIGH);
  delay(330);
  digitalWrite(10, LOW);

  Serial.begin(9600);

  if (k == 1) {
    Serial.print(flexVals[0]);
    Serial.print(",");
    Serial.print(flexVals[1]);
    Serial.print(",");
    Serial.print(flexVals[2]);
    Serial.print(",");
    Serial.print(flexVals[3]);
    Serial.print(",");
    Serial.print(flexVals[4]);
    Serial.print(",");
    Serial.println();
  }


  else if (k == 2) {

    for (int i = 0; i < 15; i++) {
      Serial.print(ACXhist[i]);
      Serial.print(",");
    }
    Serial.println();

  }

  else if (k == 3) {

    for (int i = 0; i < 15; i++) {
      Serial.print(ACYhist[i]);
      Serial.print(",");
    }
    Serial.println();
  }

  else if (k == 4) {

    for (int i = 0; i < 15; i++) {
      Serial.print(ACZhist[i]);
      Serial.print(",");
    }
    Serial.println();
  }

  else if (k == 5) {

    for (int i = 0; i < 15; i++) {
      Serial.print(GYXhist[i]);
      Serial.print(",");
    }
    Serial.println();
  }

  else if (k == 6) {

    for (int i = 0; i < 15; i++) {
      Serial.print(GYYhist[i]);
      Serial.print(",");
    }
    Serial.println();
  }

  else if (k == 7) {

    for (int i = 0; i < 15; i++) {
      Serial.print(GYZhist[i]);
      Serial.print(",");
    }
    Serial.println();
    k = 0;
    Serial.end();
  }

}

//all the interrupt actually has to do is flag that it has happened no matter where in the code it comes in
//just changing a flag ensures no problems with sets of data
void ISR0() {
  dumpTrigger = true;
}





