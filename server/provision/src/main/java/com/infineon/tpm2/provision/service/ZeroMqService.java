package com.infineon.tpm2.provision.service;

import com.infineon.tpm2.provision.configuration.ZeroMqConfig;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;

@Service
public class ZeroMqService {

    @Autowired
    private ZeroMqConfig zeroMqConfig;

    /*@Scheduled(fixedRate = 1000)
    public void sendMessage() {
        String topic = "topic";
        String message = "Broadcast from Spring Integration";
        zeroMqConfig.publish(topic, message);
    }*/

    @ServiceActivator(inputChannel = "subscribeChannel")
    public void handleMessage(Message<String> message) {
        String payload = message.getPayload();
        System.out.println("Received : " + payload);
    }
}
