#include <DualVNH5019MotorShield.h>
#include <PinChangeInterrupt.h>
#include <PID_v1.h>
#include "src/SharpIR-master/SharpIR.h"
#include <RunningMedian.h>
#include <math.h>
#include <String.h>
#include <EnableInterrupt.h>

#define PIN_LEFT_ENC 3 //left motor encoder channel A to pin 3
#define PIN_RIGHT_ENC 11 //right motor encoder channel A to pin 11


DualVNH5019MotorShield md;

// 6.04V - perfect straight

//counters for the number of ticks from each encoder
double LTicks = 0, RTicks = 0;
double startLTicks, startRTicks;

//time parameters to calculate RPM
volatile unsigned long LStart, LStop, LTimeWidth, RStart, RStop, RTimeWidth;

//set speeds for left and right wheels, and PID output correction
double RSet = 300, correction = 0, LSet;

//rpm to check if PID correction is needed
double Lrpm = 0.0, Rrpm = 0.0;

//PID params
double kp = 5.0, ki = 0, kd = 0.15; // Batt 6.1V

bool stationary;
bool exploration = false;

RunningMedian rm = RunningMedian(25);
RunningMedian rm2 = RunningMedian(5);
RunningMedian rm3 = RunningMedian(5);

//PID controller variable
PID controller(&RTicks, //input
               &correction, //output
               &LTicks, //setpoint -> left wheel is the master, PID will work to tune right to match the speed of left
               kp,
               ki,
               kd,
               DIRECT);

//creating sensor objects

// inputs:
/// 1. Pin Number, 
/// 2. Num of Readings, 
/// 3. Tolerance - % of which the value has to be similar to next value to be considered valid (>= 93),
/// 4. Offset - Modify depending on sensor calibration
/// 5. Model (1080 for GP2Y0A21Y, 20150 for GP2Y0A02YK)

SharpIR sharp1(A0, 1, 95, 0, 1080);
SharpIR sharp2(A1, 1, 95, 0, 1080);
SharpIR sharp3(A2, 1, 95, 0, 1080);
SharpIR sharp4(A3, 1, 95, 0, 1080);
SharpIR sharp5(A4, 1, 95, 0, 1080);
SharpIR sharp6(A5, 1, 95, 0, 20150);

void setup() {
  Serial.begin(57600);
  Serial.setTimeout(0);

  // set up motor & PID
  md.init();
  
  pinMode(PIN_RIGHT_ENC, INPUT);
  enableInterrupt(PIN_RIGHT_ENC, countRight, RISING);
  
  pinMode(PIN_LEFT_ENC, INPUT);
  enableInterrupt(PIN_LEFT_ENC, countLeft, RISING);
  
  controller.SetOutputLimits(-10, 10);
  controller.SetMode(AUTOMATIC);

  //set sensor pins
  pinMode (A0, INPUT);
  pinMode (A1, INPUT);
  pinMode (A2, INPUT);
  pinMode (A3, INPUT);
  pinMode (A4, INPUT);
  pinMode (A5, INPUT);

  LSet = RtoLCruise(RSet);
}

void loop() {


//========================
// A. RPI COMMS LOOP:
//------------------------
  char com;
 
  if(Serial.available()>0)
  {
    com=char(Serial.read());
    readCommands(com);
  }


//========================
// B. EXPLORATION MOVEMENT LOOP:
//------------------------

//forward(10,true);
//delay(1000);
//alignLeft();

//========================


//========================
// C. SENSOR TESTING LOOP:
//------------------------
//
//  ir_sense1();
//  ir_sense2();
//  ir_sense3();
//  ir_sense4();
//  ir_sense5();
//  ir_sense6();
//  sendSensorData();
//  shutdown();
//  delay(1000);
}

//===================================
// ROBOT COMMANDS
//===================================
void readCommands(char command)
{
  switch(command)
  {
    //Exploration
    case 'E':
      exploration = true;
      break;
      
    //Move forward by 10cm
    case 'F': 
      forward(10,true);
      break;
    
    //Rotate right by 90 degree
    case 'L':
      rotateLeft(90);
      break;

    //Rotate Left by 90 degree
    case 'R':
      rotateRight(90);
      break;
      
    // Align Left
    case 'A':
      alignLeft();
      break;
      
    // Align front
    case 'B':
      alignFront();
      break;

    // InCorner Combo
    case 'I':
      inCorner();
      break;

    // OutCorner Combo
    case 'O':
      outCorner();
      break;
      

//    //Fastest Path
//    case 'P':
//      forward(10);
//      break;
//      


  }
  sendSensorData();
  
}

void sendSensorData(){
  String sensorData = "ALG|SENSOR|{\"BL\":" + getBlockEstimate(1) + 
                          ",\"FL\":" + getBlockEstimate(2) +
                          ",\"CL\":" + getBlockEstimate(3) +
                          ",\"BC\":" + getBlockEstimate(4) +
                          ",\"BR\":" + getBlockEstimate(5) +
                          ",\"LR\":" + getBlockEstimate(6) + 
                          "}#";
                          
  Serial.println(sensorData);
}


//====================================
// BASIC MOVEMENT FUNCTIONS:
//==================================== 

// -----------------------------------
// 1. Forward
// -----------------------------------

void forward(double dist, bool explore){
  double fwd_L_encoder = LTicks;
  double fwd_R_encoder = RTicks;

  int eBrake = 5;
  
  stationary = true;
  
  if (checkForwardPossible(dist)){
    double fwd_dist;

    LTicks = RTicks;
    double last_tick_R = RTicks;
    
    double correctionMulti;

    if (explore){
//      Serial.print("Explore");
      fwd_dist = (((dist/(2*3*3.141592)) * (562.25))*0.95);
      correctionMulti = 0.6;
    }
    else{
      fwd_dist = dist/(2*3*3.141592) * (562.25*10)/13.5;
      correctionMulti = 0.9775;
    }

    LSet = RtoLCruise(RSet);

    while ((LTicks < fwd_L_encoder + fwd_dist) || (RTicks < fwd_R_encoder + fwd_dist)) {

      if(stationary && !explore){
          Serial.print("Ramp");
          rampUp();
          stationary = false;
         }
        
        if(controller.Compute()) {
          if (correction > 0){
            md.setSpeeds(RSet + correction, LSet);
          }
          else if (correction < 0){
            md.setSpeeds(RSet, LSet - correction * correctionMulti); // 1.14 for 6.05V
          }
          else{
            md.setSpeeds(RSet, LSet);
          } 
        }
        
      if (!explore && !checkForwardPossible(eBrake)) {
        md.setBrakes(400, 400);
        correctFrontSpace();
        break;
      }
    }
    md.setBrakes(400, 400);
    stationary = true;
  }
    
  else{
    inCorner();
  }

  LTicks = RTicks;
}

// -----------------------------------
// 2. Rotations
// -----------------------------------

void rotateRight(double angle) {
  LTicks = RTicks;
  startRTicks = RTicks;
  
  double target_Tick = 0;

  target_Tick = ((angle/180)*3.141592 * 8.45)/(2*3*3.141592) * 562.25;

  while (RTicks < startRTicks + target_Tick ) {
    if (controller.Compute()){
      md.setSpeeds(-(RSet+correction), LSet);
    }
  }
  md.setBrakes(400, 400);
  delay(100);

  LTicks = startRTicks;
  RTicks = startRTicks;
}

// -----------------------------------

void rotateLeft(double angle) {
  LTicks = RTicks;
  startRTicks = RTicks;
  
  double target_Tick = 0;

  target_Tick = ((angle/180)*3.141592 * 8.18)/(2*3*3.141592) * 562.25;
  
  while (RTicks < startRTicks + target_Tick ) {
    if (controller.Compute()){
//      md.setSpeeds(RSet+correction, -(LSet));
      md.setSpeeds(RSet, -(LSet-correction));
    }
  }
  md.setBrakes(400, 400);
  delay(100);

  LTicks = startRTicks;
  RTicks = startRTicks;
}

// -----------------------------------
// 3. Ramp Up
// -----------------------------------

void rampUp(){
  int LBegin;
  
  LBegin = 80;
  
  double LInc = (LSet-LBegin)/100;
  double RInc = (RSet-50)/100;

  for (int i = 0; i < 100; i++){
    md.setSpeeds(50+(i+1)*RInc, LBegin+(i+1)*LInc);
    delay(2);
  }
}

//====================================
// ADVANCED MOVEMENT FUNCTIONS:
//==================================== 

// -----------------------------------
// 1. Alignment
// -----------------------------------

void alignLeft() {

  // ==== Parameters (Edit here) ====
  double validDist = 12;
  double minL = 4.5;
  double maxL = 6;
  double maxDiff = 0.2;
  int calSpeed = 60;
  // --------------------------------
  
  double store_L_encoder = LTicks;
  double store_R_encoder = RTicks;

  double sensor_L_dis;
  double sensor_R_dis;

  double sensorDiff;

  sensor_L_dis = ir_sense1();
  sensor_R_dis = ir_sense2();
  
  if (sensor_L_dis < validDist && sensor_R_dis < validDist){

    if (sensor_L_dis < minL|| sensor_R_dis < minL || sensor_L_dis > maxL || sensor_R_dis > maxL){
      correctLeftSpace();
      alignLeft();
    }

    else{
      sensorDiff = abs(sensor_R_dis - sensor_L_dis);
    
      while (sensorDiff > maxDiff) {
        if (sensor_L_dis > sensor_R_dis) {
          md.setSpeeds(-calSpeed, calSpeed);
        }
        else if (sensor_R_dis > sensor_L_dis) {
          md.setSpeeds(calSpeed, -calSpeed);
        }
    
        sensor_L_dis = ir_sense1();
        sensor_R_dis = ir_sense2();
        sensorDiff = abs(sensor_R_dis - sensor_L_dis);
      }
    }
  }

  md.setBrakes(400, 400);

  LTicks = store_L_encoder;
  RTicks = store_R_encoder;
}

void alignFront() {

  // ==== Parameters (Edit here) ====
  double validDist = 12;
  double maxDiff = 0.3;
  int calSpeed = 60;
  // --------------------------------
  
  double store_L_encoder = LTicks;
  double store_R_encoder = RTicks;

  double sensor_L_dis;
  double sensor_M_dis;
  double sensor_R_dis;

  double sensorDiff;

  sensor_L_dis = ir_sense3();
  sensor_M_dis = ir_sense4();
  sensor_R_dis = ir_sense5();

  //-----------------------------------------------------------------------------------------

  if (sensor_L_dis < validDist && sensor_R_dis < validDist){
    
    sensorDiff = abs(sensor_R_dis - sensor_L_dis);
    
    while (sensorDiff > maxDiff) {
      if (sensor_L_dis > sensor_R_dis) {
        md.setSpeeds(-calSpeed, calSpeed);
      }
      else if (sensor_R_dis > sensor_L_dis) {
        md.setSpeeds(calSpeed, -calSpeed);
      }

      sensor_L_dis = ir_sense3();
      sensor_R_dis = ir_sense5();
      
      sensorDiff = abs(sensor_R_dis - sensor_L_dis);
    }
  }

  //-----------------------------------------------------------------------------------------

  else if (sensor_L_dis < validDist && sensor_M_dis < validDist){

    sensorDiff = abs(sensor_M_dis - sensor_L_dis);
    
    while (sensorDiff > maxDiff) {
      if (sensor_L_dis > sensor_M_dis) {
        md.setSpeeds(-calSpeed, calSpeed);
      }
      else if (sensor_M_dis > sensor_L_dis) {
        md.setSpeeds(calSpeed, -calSpeed);
      }
  
      sensor_L_dis = ir_sense3();
      sensor_M_dis = ir_sense4();
      
      sensorDiff = abs(sensor_M_dis - sensor_L_dis);
    }
  }

  //-----------------------------------------------------------------------------------------
  
  else if (sensor_M_dis < validDist && sensor_R_dis < validDist){

    sensorDiff = abs(sensor_R_dis - sensor_M_dis);
    
    while (sensorDiff > maxDiff) {
      if (sensor_M_dis > sensor_R_dis) {
        md.setSpeeds(-calSpeed, calSpeed);
      }
      else if (sensor_R_dis > sensor_M_dis) {
        md.setSpeeds(calSpeed, -calSpeed);
      }
      
      sensor_M_dis = ir_sense4();
      sensor_R_dis = ir_sense5();
      
      sensorDiff = abs(sensor_R_dis - sensor_M_dis);
    }
  }

  //-----------------------------------------------------------------------------------------
  
  else{
    return;
  }

  //-----------------------------------------------------------------------------------------

  md.setBrakes(400, 400);
    
  LTicks = store_L_encoder;
  RTicks = store_R_encoder;
}


// -----------------------------------
// 2. Correct Distances
// -----------------------------------

void correctLeftSpace(){
  rotateLeft(90);
  alignFront();
  correctFrontSpace();
  alignFront();
  rotateRight(90);
  delay(100);
}

void correctFrontSpace()
{
  // ==== Parameters (Edit here) ====
  double validDist = 11;
  double minF = 3.7;
  double maxF = 3.8;
  int calSpeed = 60;
  // --------------------------------
  
  double store_L_encoder = LTicks;
  double store_R_encoder = RTicks;

  LTicks = store_L_encoder;
  RTicks = store_R_encoder;
  
  double sensor_L_dis;
  double sensor_M_dis;
  double sensor_R_dis;

  double ave;

  sensor_L_dis = ir_sense3();
  sensor_M_dis = ir_sense4();
  sensor_R_dis = ir_sense5();

  //=========================================================================================  
  // Use 2 sensors:
  //-----------------------------------------------------------------------------------------
  if (sensor_R_dis <= validDist && sensor_L_dis <= validDist){   // LMR < 11 & LR < 11 //
    ave = (sensor_R_dis + sensor_L_dis)/2;

    while (ave < minF || ave > maxF) {
      if (ave < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      
      sensor_L_dis = ir_sense3();
      sensor_R_dis = ir_sense5();
      ave = (sensor_R_dis + sensor_L_dis)/2;
    }
  }
  //-----------------------------------------------------------------------------------------
  else if (sensor_R_dis <= validDist && sensor_M_dis <= validDist){   // MR < 11 //
    ave = (sensor_R_dis + sensor_M_dis)/2;

    while (ave < minF || ave > maxF) {
      if (ave < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      
      sensor_M_dis = ir_sense4();
      sensor_R_dis = ir_sense5();
      ave = (sensor_R_dis + sensor_M_dis)/2;
    }
  }
  //-----------------------------------------------------------------------------------------
  else if (sensor_M_dis <= validDist && sensor_L_dis <= validDist){   // LM < 11 //
    ave = (sensor_M_dis + sensor_L_dis)/2;

    while (ave < minF || ave > maxF) {
      if (ave < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      
      sensor_L_dis = ir_sense3();
      sensor_M_dis = ir_sense4();
      ave = (sensor_M_dis + sensor_L_dis)/2;
    }
  }

  //=========================================================================================  
  // Use 1 sensor:
  //-----------------------------------------------------------------------------------------
  else if (sensor_L_dis <= validDist){   // L < 11 //
    while (sensor_L_dis < minF || sensor_L_dis > maxF) {
      if (sensor_L_dis < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      sensor_L_dis = ir_sense3();
    }
  }
  //-----------------------------------------------------------------------------------------
  else if (sensor_R_dis <= validDist){   // R < 11 //
    while (sensor_R_dis < minF || sensor_R_dis > maxF) {
      if (sensor_R_dis < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      sensor_R_dis = ir_sense5();
    }
  }
  //-----------------------------------------------------------------------------------------
  else if (sensor_M_dis <= validDist){   // M < 11 //
    while (sensor_M_dis < minF || sensor_M_dis > maxF) {
      if (sensor_M_dis < minF) {
        md.setSpeeds(-calSpeed, -calSpeed);
      }
      else {
        md.setSpeeds(calSpeed, calSpeed);
      }
      sensor_M_dis = ir_sense4();
    }
  }
  //-----------------------------------------------------------------------------------------
  else{
    //cannot correct;
    return;
  }
  
  md.setBrakes(400, 400);
  LTicks = store_L_encoder;
  RTicks = store_R_encoder;
}

// -----------------------------------
// 3. Check Future Movement Possible:
// -----------------------------------

boolean checkForwardPossible(double dist){
  double sensor_L_dis;
  double sensor_M_dis;
  double sensor_R_dis;

  if (dist > 20){
    dist = 20;
  }

  sensor_L_dis = ir_sense3();
  sensor_M_dis = ir_sense4();
  sensor_R_dis = ir_sense5();
  
  if (sensor_L_dis > dist  && sensor_M_dis > dist  && sensor_R_dis > dist){
    return true;
  }
  else{
    return false;
  }
}

// -----------------------------------
// 4. Cornering
// -----------------------------------
void inCorner(){
  correctFrontSpace();
  alignLeft();
  rotateRight(90);
}

void outCorner(){
  forward(10,true);
  rotateLeft(90);

  if (checkForwardPossible(10) == true){
    forward(10,true);
    alignLeft();
  }
  
  else{
    rotateRight(90);
    alignLeft();
  }
}



//====================================
// SENSOR FUNCTIONS:
//==================================== 

// -----------------------------------
// 1. Get Distance
// -----------------------------------

//lower limit: 5, upper limit: 30 for sensor1

double ir_sense1() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp1.distance());
    }    
  
    double result = rm.getMedian() - 2.175;

    if (result < 8)
    {
      result -= 0.28; 
    }
    else if (result < 13)
    {
      result -= 0.1;
    }
    else if (result < 18)
    {
      result -= 0.15;
    }
    else if (result < 23)
    {
      result -= 0.2;
    }
    else if (result < 28){
      result -= 0.4;
    }
    else if (result < 33){
      result -= 0;
    }
    else {
      result -= 2.5;
    }
    
    rm.clear();
//      printDouble(result,100);
    rm2.add(result);
  }

  double finalReading = rm2.getMedian();
  rm2.clear();
//  printDouble(finalReading,100);
 
  return finalReading;
}

double ir_sense2() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp2.distance());
    }
    double result = rm.getMedian() - 2.2;

    if (result < 8)
    {
      result -= 0.2; 
    }
    else if (result < 13)
    {
      result -= 0.35;
    }
    else if (result < 18)
    {
      result -= 1.1;
    }
    else if (result < 23)
    {
      result -= 2;
    }
    else if (result < 28){
      result -= 3.5;
    }
    else if (result < 33){
      result -= 5;
    }
    else {
      result += 0;
    }

    rm.clear();
//      printDouble(result,100);
    rm2.add(result);
  }
  
  double finalReading = rm2.getMedian();
  rm2.clear();
//    printDouble(finalReading,100);
  
  return finalReading;
}

//upper limit: TBC lower limit:3
double ir_sense3() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp3.distance());
    }

    double result = rm.getMedian() - 6.5;

    if (result < 8)
    {
      result -= 0.6;
    }
    else if (result < 13)
    {
      result -= 1.2;
    }
    else if (result < 18)
    {
      result -= 2;
    }
    else if (result < 23)
    {
      result -= 3;
    }
    else if (result < 28){
      result -= 3.5;
    }
    else if (result < 33){
      result -= 0;
    }
    else {
      result -= 0;
    }

    rm.clear();
//    printDouble(result,100);
    rm2.add(result);
  }
  
  double finalReading = rm2.getMedian();
  rm2.clear();
//  printDouble(finalReading,100);
  
  return finalReading;
}


// BAD SENSOR
double ir_sense4() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp4.distance());
    }
  
    double result = rm.getMedian() - 2.0; //3
    
    if (result < 8){
      result -= 0.6;
    }
    else if (result < 13){
      result -= 0.9;
    }
    else if (result < 18){
      result -= 1.7;
    }
    else if (result < 23){
      result -= 2.5;
    }
    else if (result < 28){
      result -= 3.5;
    }
    else{
      result -= 4.5;
    }
    
    rm.clear();
    //  printDouble(result,100);
    rm2.add(result);
  }
  
  double finalReading = rm2.getMedian();
  rm2.clear();
//  printDouble(finalReading,100);
  
  return finalReading;
}

double ir_sense5() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp5.distance());
    }
  
    double result = rm.getMedian() - 7;
    if (result < 8){
      result -= 0.7;
    }
    else if (result < 13){
      result -= 1.5;
    }
    else if (result < 18){
      result -= 2;
    }
    else if (result < 23){
      result -= 2.3;
    }
    else if (result < 28){
      result -= 3.5;
    }
    else{
      result -= 0;
    }
    
    rm.clear();
    //  printDouble(result,100);
    rm2.add(result);
  }
  
  double finalReading = rm2.getMedian();
  rm2.clear();
//  printDouble(finalReading,100);
  
  return finalReading;
}

double ir_sense6() {
  for(int j = 0; j < 5; j++) {
    for (int i=0 ; i < 25; i++){
      rm.add(sharp6.distance());
    }
    double result = rm.getMedian() - 7.5;
    
    if (result < 18){
      result -= 1.2;
    }
    else if (result < 23){
      result -= 0;
    }
    else if (result < 28){
      result += 0.8;
    }
    else if (result < 33){
      result += 0.8;
    }
    else if (result < 38){
      result += 0.1;
    }
    else if (result < 43){
      result -= 0.5;
    }
    else if (result < 48){
      result -= 1.3;
    }
    else if (result < 53){
      result -= 1;
    }
    else if (result < 58){
      result -= 2.5;
    }
    else if (result < 63){
      result -= 2.8;
    }
    else if (result < 68){
      result -= 3.5;
    }
    else if (result < 73){
      result -= 2;
    }
    
    rm.clear();
    //  printDouble(result,100);
    rm2.add(result);
  }
  
  double finalReading = rm2.getMedian();
  rm2.clear();
//  printDouble(finalReading,100);
  
  return finalReading;
}

// -----------------------------------
// 2. Sensor Calibration
// -----------------------------------

void cal_sensor(int sensor, double actual, double times){
  
  double sum = 0;

  double cur;

  for (int i = 0; i < times; i++){
    switch(sensor){
      case 1:
          cur = ir_sense1();
          break;
        
      case 2:
          cur = ir_sense2();
          break;
          
      case 3:
          cur = ir_sense3();
          break;
      case 4:
          cur = ir_sense4();
          break;
          
      case 5:
          cur = ir_sense5();
          break;
          
//      case 6:
//          cur = ir_sense6();
//          break;
    }

    sum += cur;
  }

  double ave = sum/times;
  double diff = ave - actual;
  
 // printDouble(diff, 4);
}

// -----------------------------------
// 3. Dist to Block
// -----------------------------------

int distToBlock(double dist, int type){
  double adjusted = dist;
  
  switch(type){
    case 1: // left
      adjusted -= 5;
      break;

    case 2: // front
      adjusted -= 8;
      break;

    case 3:
      adjusted -= 5; // right
      break;
  }
  
  return round(adjusted/10);
}


// -----------------------------------
// 4. Function for Algo Conversion & Comms
// -----------------------------------

String getBlockEstimate(int sensor){
  double result;
  
  for (int i = 0; i < 5; i++){
    switch(sensor)
    {
      case 1: 
        result = ir_sense1();
        rm3.add(distToBlock(result,1));
        break;

      case 2: 
        result = ir_sense2();
        rm3.add(distToBlock(result,1));
        break;

      case 3: 
        result = ir_sense3();
        rm3.add(distToBlock(result,2));
        break;

      case 4: 
        result = ir_sense4();
        rm3.add(distToBlock(result,2));
        break;

      case 5: 
        result = ir_sense5();
        rm3.add(distToBlock(result,2));
        break;

      case 6: 
        result = ir_sense6();
        rm3.add(distToBlock(result,3));
        break;
    }
  }

  int block = rm3.getMedian(); 
  rm3.clear();

  return String(block);
}

//====================================
// ENCODER FUNCTIONS:
//==================================== 

// -----------------------------------
// 1. Increasing Encoders
// -----------------------------------

void countLeft() {
  if((int)LTicks % 50 == 0) {
   // Serial.print("[LEFT]50 ticks have passed. ");    
    getRpmLeft();
  }
  LTicks++;
}


void countRight() {
  if((int)RTicks % 50 == 0) {
    //Serial.print("[RIGHT]50 ticks have passed. ");
    getRpmRight();
  }
  RTicks++;
}


// -----------------------------------
// 2. Converting RSet to corresponding LSet
// -----------------------------------

int RtoLCruise(int Rset){
  double M1RPM = 0.3181*Rset - 10.458;
  double M2RPM = M1RPM;

  int Lset = 0;

  if (exploration){
    Lset = (M2RPM + 6.4767) / 0.335;
  }
  else{
    Lset = (M2RPM + 6.4767) / 0.397;
  }
  

//  Lset = (M2RPM + 6.4767) / 0.42; ** spd = 200
  return Lset;

  //NEW
  // M1: y = 0.3181x - 10.458
  // M2: y = 0.3471x - 6.4767
}


// -----------------------------------
// 3. Get RPMs From Encoder Values
// -----------------------------------

void getRpmLeft() {
  LStop = micros();  
  LTimeWidth = LStop - LStart; //calculate the time taken for 100 ticks

  //calculate RPM
  double LPulseWidth = LTimeWidth/LTicks;
  Lrpm = (60*1000000)/(LPulseWidth * 562.25);
  
 // Serial.print(" ;; Left RPM = ");
  //Serial.println(Lrpm, 4);
}


void getRpmRight() {
  RStop = micros();  
  RTimeWidth = RStop - RStart; //calculate the time taken for 100 ticks  

  //calculate RPM
  double RPulseWidth = RTimeWidth/RTicks;
  Rrpm = (60*1000000)/(RPulseWidth * 562.25);

 // Serial.print(" ;; Right RPM = ");
  //Serial.println(Rrpm, 4);
}


//====================================
// ADMIN FUNCTIONS:
//==================================== 

// -----------------------------------
// 1. Printing
// -----------------------------------

void printDouble(double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * precision;
   else
       frac = (int(val)- val ) * precision;
   Serial.println(frac,DEC) ;
} 


// -----------------------------------
// 2. Shutdown
// -----------------------------------

void shutdown()
{
  while (1);
}
