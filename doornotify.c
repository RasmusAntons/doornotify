#include "MQTTClient.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <MQTTClient.h>
#include "config.h"
#include "notification.h"

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_deliveryToken delivery_token;

void conn_lost(void *context, char *cause)
{
	printf("MQTT connection lost: %s\n", cause);
	while (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
		puts("MQTT: Cannot connect");
		sleep(5);
	}
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	puts("MQTT message delivered");
}

void open_front_door()
{
	MQTTClient_publish(client, MQTT_TOPIC_DOOROPEN, 4, "true", 0, 0, &delivery_token);
}

int msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	if (strcmp(MQTT_TOPIC_DOORBELL, topicName) == 0) {
		notification_show();
	} else {
		puts("MQTT: Invalid topic");
	}

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

int main(int argc, char **argv)
{
	char client_id[24];
	strcpy(client_id, "dntfy_");
	gethostname(client_id + 6, 18);
	client_id[23] = '\0';

	notification_init(client_id, open_front_door);

	MQTTClient_create(&client, MQTT_SERVER_URI, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 60;
	conn_opts.cleansession = 1;
	MQTTClient_setCallbacks(client, NULL, conn_lost, msg_arrvd, delivered);
	if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
		puts("MQTT: Cannot connect");
		return EXIT_FAILURE;
	}
	MQTTClient_subscribe(client, MQTT_TOPIC_DOORBELL, 1);

	notification_wait();

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	notification_uninit();

	return EXIT_SUCCESS;
}