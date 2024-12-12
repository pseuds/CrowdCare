# CrowdCare
Real-time public health monitoring system for 50.046 Cloud Computing & Internet of Things Project.

## Members
- 1006281 Bundhoo Simriti 
- 1005255 Swarangi Mehta
- 1006365 Ruan Yang
- 1006381 Ching Xin Wei

## ESP32 
- `esp32_sensors.ino` reads sensor inputs (temperature, humidity, motion) and publishes the data to the MQTT broker.
- `esp32_subscriber.ino` subscribes to the specified topics to retrieve sensor data, and activates the actuators accordingly.