//Dark Lord

#include <SPI.h>
#include <RF22.h>

// Singleton instance of the radio
RF22 rf22;

//Constants
int pot1pin = A0;
int pot2pin = A1;
int change_threshold = 100;

//Potentiometer variables
int pot1; 
int pot2;

//Relevant times
unsigned long lastPotCheck = 0;
unsigned long startSend = 0;
unsigned long lastSent = 0;

void setup() {

  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
}

void loop() {
  
  if (abs(millis() - lastPotCheck) > 2000) {
    
    lastPotCheck = millis();

    if (potsPastThreshold(pot1, pot2)) {
      pot1 = analogRead(pot1pin);
      pot2 = analogRead(pot2pin);

      startSend = lastPotCheck; //Current time
    }
  }
  
  if (abs(millis() - startSend) < 60000) {
    if (abs(millis() - lastSent) > 1000) {
      lastSent = millis();
      sendOnce(pot1, pot2);
    }
  }
}

bool potsPastThreshold(int first, int second) {
  int change1 = abs(first - analogRead(pot1pin));
  int change2 = abs(second - analogRead(pot2pin));

  return ((change1 + change2) > change_threshold);
}

void sendOnce(int time, int date) {
  int daysFrom = map(date, 0, 1023, 0, 7);
  int hours = map(time, 0, 1023, 0, 23);
  int minutes = map(time / 24, 0, 1024/24, 0, 3);

  uint8_t data[] = { daysFrom, hours, minutes };

  //Wait if previous packets need to be sent
  rf22.waitPacketSent();
  rf22.send(data, sizeof(data));
}
