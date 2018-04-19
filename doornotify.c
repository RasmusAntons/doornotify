#include "MQTTClient.h"
#include "config.h"
#include <libnotify/notify.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <MQTTClient.h>

void conn_lost(void *context, char *cause)
{
	printf("MQTT connection lost: %s\n", cause);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	puts("MQTT message delivered");
}

int msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	NotifyNotification *notification;
	char *action;

	switch (((char *) message->payload)[0]) {
		case 't':
			notification = notify_notification_new("Door opened", NULL, "security-low-symbolic");
			break;
		case 'f':
			notification = notify_notification_new("Door closed", NULL, "security-high-symbolic");
			break;
		default:
			perror("Invalid door action");
			exit(EXIT_FAILURE);
	}

	notify_notification_show(notification, NULL);
	g_object_unref(G_OBJECT(notification));

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

int main()
{
	MQTTClient  client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	sigset_t sigset;

	notify_init(MQTT_CLIENT_ID);

	MQTTClient_create(&client, MQTT_SERVER_URI, MQTT_CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 60;
	conn_opts.cleansession = 1;
	MQTTClient_setCallbacks(client, NULL, conn_lost, msg_arrvd, delivered);
	if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
		puts("MQTT: Cannot connect");
		return EXIT_FAILURE;
	}
	MQTTClient_subscribe(client, MQTT_TOPIC_DOOR, 1);

	sigemptyset(&sigset);
	sigsuspend(&sigset);

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	notify_uninit();

	return EXIT_SUCCESS;
}