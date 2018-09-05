#include "MQTTClient.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <MQTTClient.h>
#include "config.h"
#include <getopt.h>
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
	char server[48];
	char topic_doorbell[24];
	char topic_dooropen[24];
	char client_id[24];
	static struct option long_options[] = {
			{"server", required_argument, 0, 's'},
			{"channel", required_argument, 0, 'c'},
			{0, 0, 0, 0}
	};
	int opt, long_index = 0;

	strcpy(server, MQTT_SERVER_URI);
	strcpy(topic_doorbell, MQTT_TOPIC_DOORBELL);
	strcpy(topic_dooropen, MQTT_TOPIC_DOOROPEN);
	strcpy(client_id, "dntfy_");
	gethostname(client_id + 6, 18);
	client_id[23] = '\0';

	while ((opt = getopt_long(argc, argv, "c:s:", long_options, &long_index)) != -1) {
		switch (opt) {
			case 's':
				if (strlen(optarg) > 47) {
					printf("invalid server: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				strcpy(server, optarg);
				break;
			case 'c': {
				int channel = atoi(optarg);
				if (channel < 0 || channel > 999) {
					printf("invalid channel id: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				sprintf(topic_doorbell + 17, "%d", channel);
				sprintf(topic_dooropen + 17, "%d", channel);
				break;
			}
			default:
				exit(EXIT_FAILURE);
		}
	}

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