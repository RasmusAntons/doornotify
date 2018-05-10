#include "MQTTClient.h"
#include "config.h"
#include <libnotify/notify.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <MQTTClient.h>
#include <gtk/gtk.h>

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_deliveryToken delivery_token;
NotifyNotification *door_notification, *doorbell_notification;

void conn_lost(void *context, char *cause)
{
	printf("MQTT connection lost: %s\n", cause);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	puts("MQTT message delivered");
}

void open_front_door(NotifyNotification *notification, char *action, gpointer user_data)
{
	MQTTClient_publish(client, "esp8266/dooropen/0", 4, "true", 0, 0, &delivery_token);
}

int msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	if (strcmp(MQTT_TOPIC_DOOR, topicName) == 0) {
		if(door_notification) {
			notify_notification_close(door_notification, NULL);
			g_object_unref(G_OBJECT(door_notification));
		}
		switch (((char *) message->payload)[0]) {
			case 't':
				door_notification = notify_notification_new("Door opened", NULL, "security-low-symbolic");
				break;
			case 'f':
				door_notification = notify_notification_new("Door closed", NULL, "security-high-symbolic");
				break;
			default:
				perror("Invalid door action");
				exit(EXIT_FAILURE);
		}
		notify_notification_show(door_notification, NULL);
	} else if (strcmp(MQTT_TOPIC_DOORBELL, topicName) == 0) {
		if(doorbell_notification) {
			notify_notification_close(doorbell_notification, NULL);
			g_object_unref(G_OBJECT(doorbell_notification));
		}
		doorbell_notification = notify_notification_new("Doorbell rings", NULL, "face-surprised-symbolic");
		notify_notification_add_action(doorbell_notification, "default", "Open Front Door", NOTIFY_ACTION_CALLBACK(open_front_door), NULL, NULL);
		notify_notification_show(doorbell_notification, NULL);
	} else {
		puts("MQTT: Invalid topic");
	}

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
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
	MQTTClient_subscribe(client, MQTT_TOPIC_DOORBELL, 1);

	gtk_main();

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	if (door_notification)
		g_object_unref(G_OBJECT(door_notification));
	if (doorbell_notification)
		g_object_unref(G_OBJECT(doorbell_notification));
	notify_uninit();

	return EXIT_SUCCESS;
}