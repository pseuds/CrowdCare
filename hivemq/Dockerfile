FROM hivemq/hivemq4:dns-4.35.0

USER root

RUN apt-get update && \
    apt-get install -y wget unzip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
 
# Create the target directory
RUN mkdir -p /opt/hivemq/extensions

# Download and unzip the file
RUN wget https://github.com/hivemq/hivemq-file-rbac-extension/releases/download/4.6.2/hivemq-file-rbac-extension-4.6.2.zip -P /tmp && \
    unzip /tmp/hivemq-file-rbac-extension-4.6.2.zip -d /opt/hivemq/extensions && \
    rm /tmp/hivemq-file-rbac-extension-4.6.2.zip

# Set working directory
WORKDIR /opt/hivemq

COPY ./config.xml /opt/hivemq/conf/config.xml

