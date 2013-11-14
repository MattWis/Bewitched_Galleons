#include <SPI.h>
#include <RF22.h>

// Singleton instance of the radio
RF22 rf22;

//set pins
int transistor = 3;
int thermistor = A0;
int motorRight = 5; //pwm
int motorLeft = 6; //pwm

//constants
int tooHot = 200; //temperature/resistance threshhold
int timeToTurn;
bool message = false;


bool firstMotor = false;
bool secondMotor = false;
bool heating = false;

uint8_t buf[RF22_MAX_MESSAGE_LEN];

//timing variables
unsigned long lastTimeIdle; 
unsigned long lastTimeRx;
unsigned long turnTime;

void setup() 
{
  pinMode(transistor, OUTPUT);
  pinMode(thermistor, INPUT);
  digitalWrite(transistor, LOW);
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  rf22.setModeIdle();
  lastTimeIdle = millis();
}
//tries to read for 10 seconds
void loop() {

  if (abs(lastTimeIdle - millis()) > 50000) {
    rf22.setModeRx();
    lastTimeIdle = 0;
    lastTimeRx = millis();
  }
  
  if ((abs(lastTimeRx - millis()) < 10000) && message == false) {
    if (rf22.available()) {
      uint8_t len = sizeof(buf);
      if (rf22.recv(buf, &len)) {
        message = true;
        firstMotor = true;
        turnTime= millis();

        Serial.println((char*)buf);
      }
    }
  }

  if (message == true) {
    if(firstMotor) {
      turnTime = turnMotorClockWise(buf[0]);
      if (motorDone()) {
        firstMotor = false;
        secondMotor = true;
        turnTime = millis();
      }
    }
    if (secondMotor) {
      turnTime = turnMotorCounter(buf[1], buf[2]);
      if (motorDone()) {
        secondMotor = false;
        heating = true;
      }
    }
    if (heating) {
      heat();
      if (heatingDone()) {
        heating = false;
        lastTimeIdle = millis();
        rf22.setModeIdle();
      }
    }
  }
}

int turnMotorClockWise(uint8_t daysFrom) { 
  analogWrite(motorRight, 10);
  digitalWrite(motorLeft, LOW);
  return 500 * (int) daysFrom;
}

int turnMotorCounter(uint8_t hourOf, uint8_t minuteOf) {
  analogWrite(motorLeft, 10);
  digitalWrite(motorRight, LOW);
  return 125*(((int) hourOf * 4) + (int) minuteOf);
}

bool motorDone() { 
  if (abs(turnTime - millis()) > timeToTurn) {
    digitalWrite(motorRight, LOW);
    digitalWrite(motorLeft, LOW);
    return true;
  }
  return false;
}

void heat() {
  digitalWrite(transistor, HIGH);
}

bool heatingDone() {
  if (analogRead(thermistor) < tooHot) {
    //as temperature increases, analog read decreases
    digitalWrite(transistor, LOW);
    return true;  
  } else {
    return false;
  } 
  
}

