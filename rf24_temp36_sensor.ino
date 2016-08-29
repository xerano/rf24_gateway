/*

    ATtiny24/44/84 Pin map with CE_PIN 2 and CSN_PIN 3
                                  +-\/-+
    nRF24L01  VCC, pin2 --- VCC  1|o   |14 GND --- nRF24L01  GND, pin1
                            PB0  2|    |13 AREF
                            PB1  3|    |12 PA1
                            PB3  4|    |11 PA2 --- nRF24L01   CE, pin3
                            PB2  5|    |10 PA3 --- nRF24L01  CSN, pin4
                            PA7  6|    |9  PA4 --- nRF24L01  SCK, pin5
    nRF24L01 MISO, pin7 --- PA6  7|    |8  PA5 --- nRF24L01 MOSI, pin6
                                  +----+
*/

#define CE_PIN 2
#define CSN_PIN 3
#define TMP36 1
#define AREF_VOLTAGE 3.3 

#include "RF24.h"

RF24 radio(CE_PIN, CSN_PIN);

byte addresses[][6] = {
  "1Node","2Node"};
unsigned long payload = 0;

int tempReading;

float readTemp(){
  tempReading = analogRead(TMP36);  
  // converting that reading to voltage, which is based off the reference voltage
  float voltage = tempReading * AREF_VOLTAGE;
  voltage /= 1024.0; 
  // now print out the temperature
  return (voltage - 0.5) * 100 ;
}

void setup() {
  // Setup and configure rf radio
  radio.begin(); // Start up the radio
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]); // Write to device address '2Node'
  radio.openReadingPipe(1,addresses[0]); // Read on pipe 1 for device address '1Node'
  analogReference(EXTERNAL);
}

void loop(void){
  float temp = readTemp();
  radio.write( &temp, sizeof(float) );
  
  // Try again 1s later
  delay(30000);
}
