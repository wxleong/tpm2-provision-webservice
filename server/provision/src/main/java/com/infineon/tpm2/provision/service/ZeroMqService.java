package com.infineon.tpm2.provision.service;

import com.infineon.tpm2.provision.configuration.ZeroMqConfig;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.event.ContextRefreshedEvent;
import org.springframework.context.event.EventListener;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;
import org.zeromq.ZMQ;
import org.zeromq.ZMQException;

@Service
public class ZeroMqService {

    @Autowired
    private ZeroMqConfig zeroMqConfig;

    @EventListener(ContextRefreshedEvent.class)
    void proxyWorker() {
        Thread workerThread = new Thread(this::runProxy);
        workerThread.setDaemon(true);
        workerThread.start();
    }

    private void runProxy() {
        try {
            ZMQ.proxy(zeroMqConfig.proxyFrontendSocket(), zeroMqConfig.proxyBackendSocket(), null);
        } catch (ZMQException e) {
            e.printStackTrace();
        }
    }

    @EventListener(ContextRefreshedEvent.class)
    void subscriptionWorker() {
        Thread workerThread = new Thread(this::runSubscription);
        workerThread.setDaemon(true);
        workerThread.start();
    }

    private void runSubscription() {
        while (!Thread.currentThread().isInterrupted()) {
            String message = zeroMqConfig.receive();
            if (message != null) {
                onMessageReceived(message);
            }
        }
    }

    private void onMessageReceived(String message) {
        String tpm_response = "80010000000a00000100"; /* For testing only */

        System.out.println("Received : " + message);
        String[] parts = message.split(" ", 3);
        zeroMqConfig.publish(parts[1], parts[0], tpm_response);
    }

    /* Self-test */
    /*@Scheduled(fixedRate = 1000)
    public void sendMessage() {
        String topic = "topic";
        String message = "Broadcast from Spring Integration";
        zeroMqConfig.publish(topic, message);
    }*/
}

