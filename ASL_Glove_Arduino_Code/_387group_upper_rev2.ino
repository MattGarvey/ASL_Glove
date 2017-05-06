//Nick Wolford: april 25
//Initial revision for the code that will run on the sensor handling arduino for sign detection
//Utilizing the MPU6050 to read in values for the position and acceleration, must be tied to 
//5 flex sensors will be measured as well 
//running logs of the data will be kept and transmittted to the secondary arduino upon interrupt from the other arduino

//TODO
//implement and test on board
//individually test interrupt schema to make sure operating as expected
//look at memory usage/ impact on performance; if affordable, double the size of memory and use as a buffer so that there is guaranteed consistant data read out
//speed test to find out if it is going fast enough for our purposes
//

  //needed for the MPU6050/Serial
  #include<Wire.h>
  #include<stdlib.h>
  #include<stdio.h>
  
  //needed afor arduino interrupts to make the digitalPinToInterrupt() work
  #define NOT_AN_INTERRUPT -1
  
  const int MPU_addr=0x68;  // I2C address of the MPU-6050
  
  //memory to hold the values read in from the sensors
  int16_t* ACXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* ACYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* ACZhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  
  int16_t* GYXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* GYYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* GYZhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  
  int16_t* flexVals = (int16_t *) malloc(sizeof(int16_t) * 5);
  
  int16_t* mpuOffsets = (int16_t *) malloc(sizeof(int16_t) * 6);
  
  int dumpTrigger = 0;
  
  
  void setup(){
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
    
    delay(2000);
    
    //block to populate the memory with intial values and also let the values from the sensor settle out.
    for(int i = 19; i >= 0; i--){
       Wire.beginTransmission(MPU_addr);
       Wire.write(0x3B);
       Wire.endTransmission(false);
       Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
       
       
       ACXhist[i] = Wire.read()<<8|Wire.read();
       ACYhist[i] = Wire.read()<<8|Wire.read();
       ACZhist[i] = Wire.read()<<8|Wire.read();
       
       GYXhist[i] = Wire.read()<<8|Wire.read();
       GYYhist[i] = Wire.read()<<8|Wire.read();
       GYZhist[i] = Wire.read()<<8|Wire.read();
       
       delay(10);
    }
    Wire.endTransmission(true);
    
    //populate flex values
    flexVals[0] = analogRead(A0);
    flexVals[1] = analogRead(A6);
    flexVals[2] = analogRead(A1);
    flexVals[3] = analogRead(A2);
    flexVals[4] = analogRead(A3);
    
    
    //here is the spot where I will need to set the initial offsets for the different values
    //may require more settling time initially
    //may take some more algorithm development, initial tests not conclusive
    int total = 0;
    for(int i = 0; i < 20; i++){
      total += (ACXhist[i] / 20);
    }
    mpuOffsets[0] = total;
    
    total = 0;
    for(int i = 0; i < 20; i++){
      total += (ACYhist[i] / 20);
    }
    mpuOffsets[1] = total;
    
    total = 0;
    for(int i = 0; i < 20; i++){
      total += (ACZhist[i] / 20);
    }
    mpuOffsets[2] = total;
    
    total = 0;
    for(int i = 0; i < 20; i++){
      total += (GYXhist[i] / 20);
    }
    mpuOffsets[3] = total;
    
    total = 0;
    for(int i = 0; i < 20; i++){
      total += (GYYhist[i] / 20);
    }
    mpuOffsets[4] = total;
    
    total = 0;
    for(int i = 0; i < 20; i++){
      total += (GYZhist[i] / 20);
    }
    mpuOffsets[5] = total;
    
    for(int i = 0; i < 6; i++){
      //Serial.print(mpuOffsets[i]);
      //Serial.print(",");
    }
    //Serial.println();
    
    
    //this is the setup for the interrupts from the other arduino.
    attachInterrupt(digitalPinToInterrupt(2), ISR0, RISING);
    
    Serial.end();
    
  }
  
  //here is where the general data collection will happen
  void loop(){
    
    if(dumpTrigger){
      dumpVals();
      dumpTrigger = false;
    }
    
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);

    flexVals[0] = analogRead(A0);
    flexVals[1] = analogRead(A6);
    flexVals[2] = analogRead(A1);
    flexVals[3] = analogRead(A2);
    flexVals[4] = analogRead(A3);
    
    for(int i = 19; i >= 0; i--){
       Wire.beginTransmission(MPU_addr);
       Wire.write(0x3B);
       Wire.endTransmission(false);
       Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
       
       ACXhist[i] = Wire.read()<<8|Wire.read();
       ACXhist[i] = ACXhist[i] - mpuOffsets[0];
       ACYhist[i] = Wire.read()<<8|Wire.read();
       ACYhist[i] = ACYhist[i] - mpuOffsets[1];
       ACZhist[i] = Wire.read()<<8|Wire.read();
       ACZhist[i] = ACZhist[i] - mpuOffsets[2];
       GYXhist[i] = Wire.read()<<8|Wire.read();
       GYXhist[i] = GYXhist[i] - mpuOffsets[3];
       GYYhist[i] = Wire.read()<<8|Wire.read();
       GYYhist[i] = GYYhist[i] - mpuOffsets[4];
       GYZhist[i] = Wire.read()<<8|Wire.read();
       GYZhist[i] = GYZhist[i] - mpuOffsets[5];
       
       delay(75);
       
    }
    
    

    Wire.endTransmission(true);
  
  }
  
  //here is where the interrupt will send the data back to the other arduino upon interrupt
  //there is a lot of serial printing here, may be necessary to look into time optimizations for this.
  void dumpVals(){
    
    Serial.begin(9600);
    
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
    
    for(int i = 0; i < 20; i++){
      Serial.print(ACXhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    for(int i = 0; i < 20; i++){
      Serial.print(ACYhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    for(int i = 0; i < 20; i++){
      Serial.print(ACZhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    for(int i = 0; i < 20; i++){
      Serial.print(GYXhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    for(int i = 0; i < 20; i++){
      Serial.print(GYYhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    for(int i = 0; i < 20; i++){
      Serial.print(GYZhist[i]);
      Serial.print(",");
    }
    Serial.println();
    
    Serial.end();
  }
  
  void ISR0(){
    dumpTrigger = true;
  }
    
  



