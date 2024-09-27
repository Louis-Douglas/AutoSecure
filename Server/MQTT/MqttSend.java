package com.louisdouglas.AutoSecure.MQTT;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class MqttSend {
    private final MqttManager.MyGateway myGateway;

    @Autowired
    public MqttSend(MqttManager.MyGateway myGateway) {
        this.myGateway = myGateway;
    }

    public void sendMessage(String message) {
        myGateway.sendToMqtt(message);
    }
}