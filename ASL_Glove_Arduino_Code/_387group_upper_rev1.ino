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
  
  //needed for avr interrupts
  //http://www.nongnu.org/avr-libc/user-manual/modules.html is super useful for actually understanding whats going on with the avr bits
  #include <avr/io.h> 
  #include <util/delay.h> 
  #include <avr/interrupt.h> 
  
  const int MPU_addr=0x68;  // I2C address of the MPU-6050
  
  //memory to hold the values read in from the sensors
  int16_t* ACXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* ACYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* ACZhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  
  int16_t* GYXhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* GYYhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  int16_t* GYZhist = (int16_t *) malloc(sizeof(int16_t) * 20);
  
  int16_t* flexVals = (int16_t *) malloc(sizeof(int16_t) * 5);
  
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
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    
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
    flexVals[1] = analogRead(A0);
    flexVals[2] = analogRead(A0);
    flexVals[3] = analogRead(A0);
    flexVals[4] = analogRead(A0);
    
    
    //here is the spot where I will need to set the initial offsets for the different values
    //may require more settling time initially
    //may take some more algorithm development, initial tests not conclusive
    
    
    //here is interrupt setup, I am hoping that this is a valid place, yamuna example has it in main, setup equvalent?
    //has to be at the end of setup so that all of the values are initialized before we try to send any data to the other arduino
    initInterrupt0();
    
  }
  
  //here is where the general data collection will happen
  void loop(){
    //right now not woerrying about continuity of the signals that are being sent out; it is likely that there will be data mixed from two sampling sets
    flexVals[0] = analogRead(A0);
    flexVals[1] = analogRead(A0);
    flexVals[2] = analogRead(A0);
    flexVals[3] = analogRead(A0);
    flexVals[4] = analogRead(A0);
    
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
       
    }
    Wire.endTransmission(true);
  
  }
  
  //here is where the interrupt will send the data back to the other arduino upon interrupt
  //there is a lot of serial printing here, may be necessary to look into time optimizations for this.
  ISR(INT0_vect){
    Serial.println(flexVals[0]);
    Serial.println(flexVals[1]);
    Serial.println(flexVals[2]);
    Serial.println(flexVals[3]);
    Serial.println(flexVals[4]);
    
    for(int i = 0; i < 20; i++){
      Serial.println(ACXhist[i]);
    }
    for(int i = 0; i < 20; i++){
      Serial.println(ACYhist[i]);
    }
    for(int i = 0; i < 20; i++){
      Serial.println(ACZhist[i]);
    }
    
    for(int i = 0; i < 20; i++){
      Serial.println(GYXhist[i]);
    }
    for(int i = 0; i < 20; i++){
      Serial.println(GYYhist[i]);
    }
    for(int i = 0; i < 20; i++){
      Serial.println(GYZhist[i]);
    }
  }
  
  //must be defined to set up the avr interrupt behaviour
  void initInterrupt0(){
    EIMSK |= (1 << INT0);
    EICRA |= (1 << ISC00);
    sei();
  }
  
  



