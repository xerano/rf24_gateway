#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
#include <mosquitto.h>
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <bcm2835.h>

#define LED_PIN RPI_GPIO_P1_11
#define PIPES 3


using namespace std;

typedef struct TinyTemp {
    float temp;
    int vcc;
};

TinyTemp tinyTemp;

RF24 radio(22,0);
struct mosquitto *mosq;

void eventLoop();

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node", "3Node"};

volatile char should_disconnect = 0;

void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
	if(should_disconnect > 0){
		mosquitto_disconnect(mosq);
		should_disconnect = 0;
	}
}

void init_mosquitto(){
    mosq = mosquitto_new("id", true, NULL);
    mosquitto_publish_callback_set(mosq, on_publish);
}

void setup_radio(){
	// Setup and configure rf radio
    radio.begin();
    // optionally, increase the delay between retries & # of retries
    radio.setRetries(15,15);
    // Dump the configuration of the rf unit for debugging
    radio.printDetails();
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    radio.openReadingPipe(2,pipes[2]);
      
    radio.startListening();
}

int main(int argc, char** argv){
	
	printf("Setup radio...\n");
    setup_radio();

    // init LED
    //printf("Initializing status LED...\n");
    //bcm2835_gpio_fsel(LED_PIN, BCM2835_GPIO_FSEL_OUTP);
    
    printf("Initializing mosquitto...\n");
    init_mosquitto();
	
    eventLoop();
}

void eventLoop(){
	while (1)
	{
		// if there is data ready
		uint8_t pipe = 1;

		//bcm2835_gpio_write(LED_PIN, LOW);

		if ( radio.available(&pipe) )
		{
			//bcm2835_gpio_write(LED_PIN, HIGH);

			// Fetch the payload, and see if this was the last one.
			while(radio.available()){
				radio.read( &tinyTemp, sizeof(tinyTemp) );
			}
							
			mosquitto_connect(mosq, "localhost", 1883, 0);
			printf("received(%d)  temp: %f vcc: %d \n", pipe, tinyTemp.temp, tinyTemp.vcc);
            
            char topic_temp[50]; 
            char topic_vcc[50];
            
            sprintf(topic_temp, "emon/%d/temp", pipe);
            sprintf(topic_vcc, "emon/%d/vcc", pipe);
            
            char temp_val[10];
            char vcc_val[10];
            sprintf(temp_val, "%.3f", tinyTemp.temp);
            sprintf(vcc_val, "%.3f", tinyTemp.vcc / 1000.0);
            
            printf("Temp: %s\n", temp_val);
            printf("VCC: %s\n", vcc_val);

			mosquitto_publish(mosq, NULL, topic_temp, strlen(temp_val), temp_val, 0, false);
			mosquitto_publish(mosq, NULL, topic_vcc, strlen(vcc_val), vcc_val, 0, false);
			should_disconnect++;

            pipe++;
			if(pipe>PIPES){
				pipe=0;
			}
		}
	  delay(100); //Delay after payload responded to, minimize RPi CPU time
	} // forever loop

}
