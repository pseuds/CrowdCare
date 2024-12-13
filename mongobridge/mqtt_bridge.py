import os
import paho.mqtt.client as mqtt
from pymongo import MongoClient
from datetime import datetime
import json
from urllib.parse import quote_plus
import logging

latest_values = {
    "sensors/temperature": None,
    "sensors/humidity": None,
    "sensors/motion": None
}

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[logging.StreamHandler()]
)
logger = logging.getLogger(__name__)

# MongoDB connection
#TODO: ENTER YOUR DB USERNAME
username = quote_plus("")
# TODO: ENTER YOUR DB PASSWORD
password = quote_plus("")
#TODO: ENTER YOUR CONNECTION URL
MONGO_URI = ""
mongo_client = MongoClient(MONGO_URI)
db = mongo_client.humidity_sensor
collection = db.alldata

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        logger.info("Successfully connected to MQTT broker")
        topics = [
            #TODO: EDIT TOPIC IF NECESSARY
            ("sensors/humidity", 0),
            ("sensors/temperature", 0),
            ("sensors/motion", 0)
        ]
        client.subscribe(topics)
        logger.info("Subscribed to topics: {topics}")
    else:
        logger.error(f"Failed to connect to MQTT broker with return code: {rc}")

def on_disconnect(client, userdata, rc):
    logger.warning(f"Disconnected from MQTT broker with result code: {rc}")

def on_message(client, userdata, msg):
    try:
        print(msg)
        payload = msg.payload.decode()
        logger.debug(f"Received message on topic {msg.topic}: {payload}")

        
            # Store the parsed value in the latest_values dictionary
        latest_values[msg.topic] = float(payload)
        logger.debug(f"Updated {msg.topic} value to {payload}")

            # Check if we have values for all topics
        if all(value is not None for value in latest_values.values()):
                # Create document with all sensor values
            document = {
                "timestamp": datetime.now(),
                "temperature": latest_values["sensors/temperature"],
                "humidity": latest_values["sensors/humidity"],
                "motion": latest_values["sensors/motion"]
            }

                # Insert document and reset values
            collection.insert_one(document)
            logger.info("Successfully stored complete sensor data")
                
                # Reset the values to None after storing
            for topic in latest_values:
                latest_values[topic] = None
    except Exception as e:
        logger.error(f"Failed to send: {str(e)}", exc_info=True)

# If this is at the module level (not inside any class or function)
def on_subscribe(client, userdata, mid, granted_qos):
    print(f"Subscribed with mid: {mid} and QoS: {granted_qos}")

# MQTT setup
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.on_subscribe = on_subscribe
mqtt_client.on_disconnect = on_disconnect

#TODO: HIVEMQ AUTHENTICATION- UNAME PASSWD
mqtt_client.username_pw_set("", "")

try:
    logger.info("Attempting to connect to MQTT broker at mqtt-broker:1883")
    #TODO: CHANGE TO HIVEMQ'S CONNECTION URL
    mqtt_client.connect("", 1883, 60)
    logger.info("Connection attempt completed, starting loop")
    mqtt_client.loop_forever()
except Exception as e:
    logger.error(f"Failed to connect to MQTT broker: {str(e)}", exc_info=True)
