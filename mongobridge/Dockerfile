FROM python:3.9-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the bridge script
COPY mqtt_bridge.py .

CMD ["python", "mqtt_bridge.py"]
