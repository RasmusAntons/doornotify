#!/usr/bin/env python3

from telegram import ReplyKeyboardMarkup
from telegram.ext import Updater, CommandHandler
import paho.mqtt.client as mqtt
import json
import time
import threading

MQTT_TOPIC_DOORBELL = "esp8266/doorbell/0"
MQTT_TOPIC_DOOROPEN = "esp8266/dooropen/0"
MQTT_TOPIC_DOOROPEN_ACK = "esp8266/dooropen/1"
CONFIG_JSON = "config.json"
SUBSCRIBERS_JSON = "subscribers.json"

with open(CONFIG_JSON, 'r') as f:
	config = json.load(f)
updater = Updater(token=config["token"])
dispatcher = updater.dispatcher
subscribers = set()
client = mqtt.Client()
last_ts = 0
missed_sigs = 0
missed_sig_timer = None
open_requests = set()

keyboard_unsubscribed = ReplyKeyboardMarkup([['/subscribe'], ['/open']])
keyboard_subscribed = ReplyKeyboardMarkup([['/unsubscribe'], ['/open']])

def missed_sig_callback():
	global missed_sigs
	for subscriber in subscribers:
		updater.bot.send_message(chat_id=subscriber, text="{} missed signals".format(missed_sigs), reply_markup=keyboard_subscribed)
	missed_sigs = 0

def store_subscribers():
	with open(SUBSCRIBERS_JSON, 'w') as f:
		json.dump(list(subscribers), f)

def on_connect(client, userdata, flags, rc):
	client.subscribe(MQTT_TOPIC_DOORBELL)
	client.subscribe(MQTT_TOPIC_DOOROPEN)
	client.subscribe(MQTT_TOPIC_DOOROPEN_ACK)
	print("connected")

def on_message(client, userdata, msg):
	global last_ts, missed_sigs, missed_sig_timer, open_requests
	if msg.topic == MQTT_TOPIC_DOORBELL:
		current_ts = time.time()
		dt = current_ts - last_ts
		last_ts = current_ts
		if dt < 1:
			missed_sigs += 1
			if missed_sig_timer is not None:
				missed_sig_timer.cancel()
			missed_sig_timer = threading.Timer(1, missed_sig_callback)
			missed_sig_timer.start()
		else:
			for subscriber in subscribers:
				updater.bot.send_message(chat_id=subscriber, text="Doorbell rings", reply_markup=keyboard_subscribed)
	elif msg.topic == MQTT_TOPIC_DOOROPEN_ACK:
		for request in open_requests:
			updater.bot.send_message(chat_id=request, text="Door opened", reply_markup=keyboard_subscribed)
		open_requests.clear()

def start(bot, update):
	keyboard = keyboard_subscribed if update.message.chat_id in subscribers else keyboard_unsubscribed
	bot.send_message(chat_id=update.message.chat_id, text="Doornotify Bot", reply_markup=keyboard)

def subscribe(bot, update):
	subscribers.add(update.message.chat_id)
	bot.send_message(chat_id=update.message.chat_id, text="You are now subscribed", reply_markup=keyboard_subscribed)
	store_subscribers()

def unsubscribe(bot, update):
	subscribers.discard(update.message.chat_id)
	bot.send_message(chat_id=update.message.chat_id, text="You are no longer subscribed", reply_markup=keyboard_unsubscribed)
	store_subscribers()

def open_door(bot, update):
	client.publish(MQTT_TOPIC_DOOROPEN, "true")
	keyboard = keyboard_subscribed if update.message.chat_id in subscribers else keyboard_unsubscribed
	bot.send_message(chat_id=update.message.chat_id, text="Opening the door", reply_markup=keyboard)
	open_requests.add(update.message.chat_id)

with open('subscribers.json') as f:
	subscribers.update(json.load(f))

start_handler = CommandHandler('start', start)
subscribe_handler = CommandHandler('subscribe', subscribe)
unsubscribe_handler = CommandHandler('unsubscribe', unsubscribe)
open_handler = CommandHandler('open', open_door)

dispatcher.add_handler(start_handler)
dispatcher.add_handler(subscribe_handler)
dispatcher.add_handler(unsubscribe_handler)
dispatcher.add_handler(open_handler)

updater.start_polling()

client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 120)
client.loop_start()

updater.idle()
client.loop_stop()
