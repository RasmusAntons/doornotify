#ifdef linux

#include "notification.h"
#include <libnotify/notify.h>
#include <glib.h>
#include <gtk/gtk.h>

static NotifyNotification *doorbell_notification;
static void (*cb)(void);


static void action_callback(NotifyNotification *notification, char *action, gpointer user_data) {
	cb();
}

void notification_init(char *app_name, void (*callback)(void))
{
	gtk_init(0, NULL);
	notify_init(app_name);
	cb = callback;
}

void notification_show()
{
	if(doorbell_notification) {
		notify_notification_close(doorbell_notification, NULL);
		g_object_unref(G_OBJECT(doorbell_notification));
	}
	doorbell_notification = notify_notification_new("Doorbell rings", NULL, "face-surprised-symbolic");
	notify_notification_add_action(doorbell_notification, "default", "Open Front Door", NOTIFY_ACTION_CALLBACK(action_callback), NULL, NULL);
	notify_notification_show(doorbell_notification, NULL);
}

void notification_wait()
{
	gtk_main();
}

void notification_uninit()
{
	notify_uninit();
	if (doorbell_notification)
		g_object_unref(G_OBJECT(doorbell_notification));
}

#endif /* linux */