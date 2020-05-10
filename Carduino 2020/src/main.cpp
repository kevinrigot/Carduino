#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <message.h>

const String APP_NAME = "CArduino 2020 - " __FILE__;
const String APP_VERSION = "v0.4-" __DATE__ " " __TIME__;
#define DEBUG 

#ifdef DEBUG
 #define IF_SERIAL_DEBUGS(x)  x;
#else
 #define IF_SERIAL_DEBUGS(x)
#endif
/*******************************************************************************************************************************************************/
/*          PIN LAYOUT        */
/*******************************************************************************************************************************************************/

#define ESC_PIN  9
#define DIRECTION_PIN 10
#define VOLTAGE_1 A0
#define VOLTAGE_2 A1


/*******************************************************************************************************************************************************/
/*          Nikko Motor  with ESC F05428        */
/*******************************************************************************************************************************************************/
const int ESC_BACKWARD_MAX = 30;
const int ESC_BACKWARD_MIN = 77;
const int ESC_STOP = 88;
const int ESC_FORWARD_MIN = 105;
const int ESC_FORWARD_MAX = 135;

const int PRECISION_Y = 10;
const int PRECISION_Y_NEG = PRECISION_Y * -1;

Servo escServo;

void Motor_setup() {
  Serial.println("Seting up motors");
  escServo.attach(ESC_PIN);
  escServo.write(ESC_STOP);
}
int previousJoyY = 0;
void move(int joyY) {
  int speed = ESC_STOP;
  if(joyY < 0){
    if(previousJoyY >= 0){
      IF_SERIAL_DEBUGS(Serial.println("Disable break"));
      escServo.write(ESC_BACKWARD_MAX - 5);
      delay(50);
      escServo.write(ESC_STOP);
      delay(50);
    }
    speed = map(joyY, PRECISION_Y_NEG, -1 , ESC_BACKWARD_MAX, ESC_BACKWARD_MIN);
  } else if(joyY > 0){
    if(previousJoyY <= 0){
      IF_SERIAL_DEBUGS(Serial.println("Disable break"));
      escServo.write(ESC_FORWARD_MAX + 5);
      delay(50);
      escServo.write(ESC_STOP);
      delay(50);
    }
    speed = map(joyY, 1, PRECISION_Y , ESC_FORWARD_MIN, ESC_FORWARD_MAX);
  }
  previousJoyY = joyY;
  IF_SERIAL_DEBUGS(Serial.print("Move: "));
  IF_SERIAL_DEBUGS(Serial.println(speed));
  escServo.write(speed);
}

void Motor_test() {

  Serial.println("Testing motors. Move forward ...");
//Move forward
  escServo.write(ESC_FORWARD_MAX);
  delay(2000);
  escServo.write(0);
  delay(2000);  
  

  Serial.println("Testing motors. Move backward ...");
//Move backward
  escServo.write(ESC_BACKWARD_MAX);
  delay(2000);
  escServo.write(0);
  delay(2000);

  Serial.println("Testing motors done.");
}

void Motor_calibrate(){
  Serial.println("Calibrating motor ");
  while(true){
    int throttle = analogRead(A0);    
    throttle = map(throttle, 0, 1023, 20, 150); //Rescale to fix
    Serial.println(throttle);
    escServo.write(throttle);
    delay(100);
  }
}


/*******************************************************************************************************************************************************/
/*          Direction - MG995 Servo        */
/* Tension: 4.8V - 6V */
/*******************************************************************************************************************************************************/

Servo dirServo;

const int SERVO_LIMIT_MIN = 45; 
const int SERVO_CENTER    = 95;
const int SERVO_LIMIT_MAX = 145;

const int PRECISION_X = 5;
const int PRECISION_X_NEG = PRECISION_X * -1;

int servoPosition = 0;
void Servo_setup() {
  Serial.println("Seting up direction");

  dirServo.attach(DIRECTION_PIN);
  dirServo.write(SERVO_CENTER);
  servoPosition = SERVO_CENTER;
}


void turn(int joyY) {
  int val = map(joyY, PRECISION_X_NEG, PRECISION_X, SERVO_LIMIT_MIN, SERVO_LIMIT_MAX);
  IF_SERIAL_DEBUGS(Serial.print("Turn: "));
  IF_SERIAL_DEBUGS(Serial.println(servoPosition - val));
  if (abs(servoPosition - val) > 5) {
    dirServo.write(val);
    servoPosition = val;
  }
}
void Servo_test() {
  Serial.println("Testing direction...");
  delay(1000);
  dirServo.write(SERVO_LIMIT_MIN);
  delay(1000);
  dirServo.write(SERVO_LIMIT_MAX);
  delay(1000);
  dirServo.write(SERVO_CENTER);
  Serial.println("Testing direction done");
}

void Servo_calibrate(){
  Serial.println("Calibrating direction");
  while(true){
    int val = analogRead(A0);      
    val = map(val, 0, 1023, SERVO_LIMIT_MIN, SERVO_LIMIT_MAX); //Rescale to fix
    Serial.println(val);
    dirServo.write(val);
    delay(50);
  }
}

/*******************************************************************************************************************************************************
          RADIO  - NRF24L01        
Library: TMRh20/RF24, https://github.com/tmrh20/RF24/ 
This is using the RF24 library using the nRF24L01+ and the Arduino Mega2560

Line    Arduino Pin
GND    GND 
3V3    3V3 
CE    7 
CSN    8 
SCK    52 
MOSI    51 
MISO    50 
*******************************************************************************************************************************************************/

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

void Radio_setup(){
  Serial.println("Seting up Radio");
  radio.begin();
  radio.openWritingPipe(addresses[0]); 
  radio.openReadingPipe(0, addresses[1]); 
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

RadioMessageResponse radio_listen(){
  if ( radio.available()) {
    RadioMessage msg = {0, 0};

    IF_SERIAL_DEBUGS(Serial.print("radio av"));
    radio.read(&msg, sizeof(msg));
    return RadioMessageResponse(msg);
  }
  return RadioMessageResponse();
}

void radio_send(RadioMessageToRemote msg){
  radio.stopListening();
  radio.write(&msg, sizeof(msg));
  radio.startListening();
}

/******************************************
 *     Battery Voltage Indicator
 ******************************************/

float getVoltage(uint8_t pin)
{
  int sensorValue = analogRead(pin); //read the A0 pin value
  return sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage.
}


void printVoltage(){
  float voltage = getVoltage(VOLTAGE_1);
  Serial.print("voltage = ");
  Serial.print(voltage); //print the voltage to LCD
  Serial.println(" V");
  voltage = getVoltage(VOLTAGE_2);
  Serial.print("voltage = ");
  Serial.print(voltage); //print the voltage to LCD
  Serial.println(" V");
}

RadioMessageToRemote getVoltageMessage(){
    IF_SERIAL_DEBUGS(printVoltage());
    RadioMessageToRemote msg = {0, 0};
    msg.voltage1 = getVoltage(VOLTAGE_1);
    msg.voltage2 = getVoltage(VOLTAGE_2);
    return msg;
}


/*******************************************************************************************************************************************************/
/*          Carduino       */
/*******************************************************************************************************************************************************/

void setup()  {
  Serial.begin(9600);
  Serial.println(APP_NAME);
  Serial.println(APP_VERSION);
  Serial.println("Seting up...");
  Radio_setup();
  Motor_setup();
  Servo_setup();
  
  //Motor_calibrate();
  // Motor_test();
  //Servo_test();
  // Servo_calibrate();
  printVoltage();
  radio_send(getVoltageMessage());
  Serial.println("Set up done.");
}
RadioMessage radioMessage;
RadioMessageResponse response;

unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 2000;  // interval at which to blink (milliseconds)
int noMouvementCounter = 0;
void loop(){
  unsigned long currentMillis = millis();
  response = radio_listen();
  if(response.isReceived()){
    radioMessage = response.getRadioMessage();
    String log = "Radio message received: JoyY: ";
    IF_SERIAL_DEBUGS(Serial.println(log + radioMessage.joystickY+" / JoyX: "+radioMessage.joystickX));
    turn(radioMessage.joystickX);    
    move(radioMessage.joystickY);    
    if(radioMessage.joystickY == 0 && radioMessage.joystickX == 0){
      noMouvementCounter++;
    }else{
      noMouvementCounter = 0;
    }
    if(noMouvementCounter>10){
      radio_send(getVoltageMessage());
      noMouvementCounter = 0;
    }
    
    previousMillis = currentMillis;
  }else{    
    if (currentMillis - previousMillis >= interval) {
      IF_SERIAL_DEBUGS(Serial.println("No messages received in the last 2000ms"));
      turn(0);    
      move(0);    
      previousMillis = currentMillis;
    }
  }
}
