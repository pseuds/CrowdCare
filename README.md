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

## Software
- K3s
- Docker
- HiveMQ Broker
- MongoDB Cloud
- Streamlit
- Cloudflare

## To run
The following needs to be set up first.
1) K3s master and worker
2) Make sure master and worker can talk to each other.
3) Create MongoDB database.
4) Edit the lines in TODO in all python scripts. Better to use env file.
5) Build the Dockerfiles first.
6) Then run `docker save my/local-image:v1.2.3 | sudo k3s ctr images import -` to build the custom images.
7) Apply the yaml scripts with `kubectl apply -f <filenames>`. Apply the Deployments first, then the services.
8) Check the status of running pods with `kubectl get pods`. Debug if neccesary with `kubectl logs <podname> -c <containername in pod> 
