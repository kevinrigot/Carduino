#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <message.h>
#include <custom-chars.h>
#include <LiquidCrystal_I2C.h>

const String APP_NAME = "CArduino Remote 2020 - " __FILE__;
const String APP_VERSION = "v0.4-" __DATE__ " " __TIME__;
//#define DEBUG 

#ifdef DEBUG
 #define IF_SERIAL_DEBUGS(x)  x;
#else
 #define IF_SERIAL_DEBUGS(x)
#endif

/*******************************************************************************************************************************************************/
/*          PIN LAYOUT        */
/*******************************************************************************************************************************************************/

const int JOY_Y_PIN = A0;
const int JOY_X_PIN = A1;

//SCL - A5
//SDA - A4
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//CE - D7
//CSN - D8
RF24 radio(7, 8);

/*******************************************************************************************************************************************************/
/*          LCD I2C  - IICI2C 1602 LCD Module        */
/* Library: LiquidCrystal_I2C */
/*******************************************************************************************************************************************************/

void setup_lcd(){
  Serial.println("Setting up LCD...");
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(APP_NAME.substring(0,15));
  lcd.setCursor(0,1);
  lcd.print(APP_VERSION);
  IF_SERIAL_DEBUGS(lcd.setCursor(11,1));
  IF_SERIAL_DEBUGS(lcd.print("DEBUG"));
  lcd.createChar(CHAR_UP, char_up);
  lcd.createChar(CHAR_DOWN, char_down);
  lcd.createChar(CHAR_LEFT, char_left);
  lcd.createChar(CHAR_RIGHT, char_right);
  delay(500);
}

void lcd_clear_line(int line){
  lcd.setCursor(0, line);
  lcd.print(F("                "));
}


/*******************************************************************************************************************************************************/
/*          RADIO  - NRF24L01        */
/* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/ */
/*******************************************************************************************************************************************************/
const byte addresses[][6] = {"00001", "00002"};

void setup_radio(){
  Serial.println("Setting up Radio...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Init radio"));
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void radio_listen(){
  if (radio.available()) {
    RadioMessageToRemote msg = {0, 0};
    radio.read(&msg, sizeof(msg));
    lcd_clear_line(0);
    lcd.setCursor(0,0);
    lcd.print(msg.voltage1);
    lcd.print("V");
    lcd.setCursor(10,0);
    lcd.print(msg.voltage2);
    lcd.print("V");
    IF_SERIAL_DEBUGS(Serial.print("Voltage values received: "));
    IF_SERIAL_DEBUGS(Serial.print(msg.voltage1));
    IF_SERIAL_DEBUGS(Serial.print("V  -  "));
    IF_SERIAL_DEBUGS(Serial.print(msg.voltage2));
    IF_SERIAL_DEBUGS(Serial.println("V"));
  }
}

void radio_ping(){
  const char text[] = "Hello World";
  Serial.println("Ping...");
  radio.write(&text, sizeof(text));
}

void radio_send(RadioMessage msg){
  radio.stopListening();
  delay(5);
  radio.write(&msg, sizeof(msg));
  IF_SERIAL_DEBUGS(Serial.print("Message: x:"));
  IF_SERIAL_DEBUGS(Serial.print(msg.joystickX));
  IF_SERIAL_DEBUGS(Serial.print(" y:"));
  IF_SERIAL_DEBUGS(Serial.println(msg.joystickY)); 
  radio.startListening();
  delay(5);
}

/*******************************************************************************************************************************************************/
/*          Joystick - 2 potentiometer        */
/*******************************************************************************************************************************************************/
void calibrate();
void setup_joystick();
void readJoystick(RadioMessage * joysticks);

int MAX_LEFT = 285;
int MAX_RIGHT = 589;
int MAX_UP = 232;
int MAX_DOWN = 524;
const int MAX_ANGLE = 300;
const int ADJUSTER = 4;
int ADJUSTED_MAX_LEFT = MAX_LEFT+ADJUSTER;
int ADJUSTED_MAX_RIGHT = MAX_RIGHT-ADJUSTER;
int ADJUSTED_MAX_UP = MAX_UP-ADJUSTER;
int ADJUSTED_MAX_DOWN = MAX_DOWN+ADJUSTER;
const int CALIBRATION_TIMEOUT = 3000;

void setup_joystick(){
  Serial.println("Setting up Joysticks...");
  pinMode(JOY_Y_PIN, INPUT);
  pinMode(JOY_X_PIN, INPUT);

  int middleX = analogRead(JOY_X_PIN);
  MAX_LEFT = middleX - (MAX_ANGLE/2);
  MAX_RIGHT = middleX + (MAX_ANGLE/2);
  int middleY = analogRead(JOY_Y_PIN);
  MAX_UP = middleY - (MAX_ANGLE/2);
  MAX_DOWN = middleY + (MAX_ANGLE/2);

  ADJUSTED_MAX_LEFT = MAX_LEFT+ADJUSTER;
  ADJUSTED_MAX_RIGHT = MAX_RIGHT-ADJUSTER;
  ADJUSTED_MAX_UP = MAX_UP-ADJUSTER;
  ADJUSTED_MAX_DOWN = MAX_DOWN+ADJUSTER;

}

const int PRECISION_X = 5;
const int PRECISION_X_NEG = PRECISION_X * -1;
const int PRECISION_Y = 10;
const int PRECISION_Y_NEG = PRECISION_Y * -1;

void readJoystick(RadioMessage * joysticks){

  joysticks->joystickX = map(analogRead(JOY_X_PIN), ADJUSTED_MAX_LEFT, ADJUSTED_MAX_RIGHT, PRECISION_X_NEG, PRECISION_X);
  joysticks->joystickY = map(analogRead(JOY_Y_PIN), ADJUSTED_MAX_UP, ADJUSTED_MAX_DOWN, PRECISION_Y, PRECISION_Y_NEG); 
  if(abs(joysticks->joystickX) == 1)joysticks->joystickX = 0;
  if(joysticks->joystickX > PRECISION_X)joysticks->joystickX = PRECISION_X;
  if(joysticks->joystickX < PRECISION_X_NEG)joysticks->joystickX = PRECISION_X_NEG;
  if(abs(joysticks->joystickY) == 1)joysticks->joystickY = 0;
  if(joysticks->joystickY > PRECISION_Y)joysticks->joystickY = PRECISION_Y;
  if(joysticks->joystickY < PRECISION_Y_NEG)joysticks->joystickY = PRECISION_Y_NEG;

  /*IF_SERIAL_DEBUGS(Serial.print("X:"));
  IF_SERIAL_DEBUGS(Serial.print(joysticks->joystickX));
  IF_SERIAL_DEBUGS(Serial.print(" Y:"));
  IF_SERIAL_DEBUGS(Serial.println(joysticks->joystickY));
  IF_SERIAL_DEBUGS(delay(100));*/
}

/*******************************************************************************************************************************************************/
/*          MAIN        */
/*******************************************************************************************************************************************************/

void setup()
{
  Serial.begin(115200);
  Serial.println(APP_NAME);
  Serial.println(APP_VERSION);
  IF_SERIAL_DEBUGS(Serial.println("DEBUG mode"));
  setup_lcd();
  setup_joystick();
  delay(500);
  setup_radio();
  Serial.println("setup done.");

  lcd.clear();
}
RadioMessage msg;
RadioMessage prevMsg;
RadioMessage joysticks;

unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;  // interval at which to blink (milliseconds)
int nbIdenticalMessage = 0;
void loop()
{
  unsigned long currentMillis = millis();
  radio_listen();
  readJoystick(&joysticks);

  msg.joystickX = joysticks.joystickX;
  msg.joystickY = joysticks.joystickY;
  if(msg.joystickY != prevMsg.joystickY || msg.joystickX != prevMsg.joystickX){ 
    nbIdenticalMessage = 0;
  }
  if(nbIdenticalMessage < 5){  
    radio_send(msg);
    prevMsg.joystickY = msg.joystickY;
    prevMsg.joystickX = msg.joystickX;
    lcd.setCursor(0, 1);
    lcd.print(joysticks.joystickX);  
    lcd.print("   ");  
    lcd.setCursor(8, 1);
    lcd.print(joysticks.joystickY);
    lcd.print("   ");  
    previousMillis = currentMillis;
    nbIdenticalMessage++;
  }else {
    if (currentMillis - previousMillis >= interval) {
      // Send a message at least every 1000ms
      radio_send(msg);
      // radio_ping();
      previousMillis = currentMillis;
    }
  }
}

