#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <sensors.h>

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	printf("ID: %d\n", * (int *) obj);
	const char *topics[2] = {"ESP1/#", "ESP2/#"};
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe_multiple(mosq, NULL, 2, (char *const *const)&topics, 0, 0, NULL);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	const char *topics[2] = {"ESP1", "ESP2"};

	//printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);

	for(int i = 0; i <= (sizeof(topics)/8); i++)
	{
		if(strstr(msg->topic, topics[i]) != NULL)
		{
			//printf("Topic found\n");
			if(strstr(msg->topic, "DHT") != NULL)
			{
				//printf("DHT Found\n");
				receive_dht(msg->topic, (char *) msg->payload);
				break;
			}
			else if(strstr(msg->topic, "BMP") != NULL)
			{
				//printf("BMP Found\n");
				receive_bmp(msg->topic, (char *) msg->payload);
				break;
			}
			else
			{
				printf("No matching sensor found\n");
			}
		}
	}
}
int main()
{
	int rc, id=12;

	mosquitto_lib_init();

	struct mosquitto *mosq;

	mosq = mosquitto_new("subscribe-test", true, &id);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, "localhost", 1883, 10);
	if(rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		return -1;
	}

	mosquitto_loop_start(mosq);
	printf("Press Enter to quit...\n");
	getchar();
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}