#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
#include <mosquitto.h>
#include <json.h>

using namespace std;
RF24 radio(22,0);
/********************************/

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
    mosquitto_disconnect(mosq);
}

int main(int argc, char** argv){     

    // Setup and configure rf radio
    radio.begin();
    // optionally, increase the delay between retries & # of retries
    radio.setRetries(15,15);
    // Dump the configuration of the rf unit for debugging
    radio.printDetails();
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
      
    radio.startListening();

    struct mosquitto *mosq;

    mosq = mosquitto_new("id", true, NULL);
    mosquitto_publish_callback_set(mosq, on_publish);

    // forever loop
    while (1)
	{
        // if there is data ready
        if ( radio.available() )
        {
            // Dump the payloads until we've gotten everything
            float temp;

            // Fetch the payload, and see if this was the last one.
            while(radio.available()){
              radio.read( &temp, sizeof(float) );
            }
            
            
            json_object * jobj = json_object_new_object();
            json_object *jdouble = json_object_new_double(temp);
            json_object_object_add(jobj,"temp", jdouble);
            
            mosquitto_connect(mosq, "localhost", 1883, 0);
            printf("Now sending %f\n", temp);
            const char *jsonString = json_object_to_json_string(jobj);
            mosquitto_publish(mosq, NULL, "sensors/temp/0", strlen(jsonString), jsonString, 0, false);
            delay(925); //Delay after payload responded to, minimize RPi CPU time
            
        }
            
    } // forever loop

    return 0;
}

