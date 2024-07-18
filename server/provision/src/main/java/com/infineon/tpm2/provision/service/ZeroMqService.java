package com.infineon.tpm2.provision.service;

import com.infineon.tpm2.provision.configuration.ZeroMqConfig;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;
import org.zeromq.ZMQ;

@Service
public class ZeroMqService {

    @Autowired
    private ZMQ.Socket publisherSocket;

    @Scheduled(fixedRate = 1000)
    public void sendMessage() {
        String message = "Broadcast from Spring Integration";
        ZeroMqConfig.publish(publisherSocket, message);
    }

    @ServiceActivator(inputChannel = "subscribeChannel")
    public void handleMessage(Message<String> message) {
        String payload = message.getPayload();
        System.out.println("Received : " + payload);
    }
}
