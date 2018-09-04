#ifndef DOORNOTIFY_NOTIFY_H
#define DOORNOTIFY_NOTIFY_H

void notification_init(char *app_name, void (*callback)(void));
void notification_show();
void notification_wait();
void notification_uninit();

#endif //DOORNOTIFY_NOTIFY_H
