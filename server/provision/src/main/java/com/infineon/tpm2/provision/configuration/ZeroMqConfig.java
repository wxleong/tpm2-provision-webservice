package com.infineon.tpm2.provision.configuration;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

@Configuration
public class ZeroMqConfig {

    @Value("${zeromq.publisher.port}")
    private int frontendPort;

    @Value("${zeromq.subscriber.port}")
    private int backendPort;

    @Bean
    ZContext proxyZContext() {
        ZContext context = new ZContext();
        return context;
    }

    @Bean
    ZContext pubZContext() {
        ZContext context = new ZContext();
        return context;
    }

    @Bean
    ZContext subZContext() {
        ZContext context = new ZContext();
        return context;
    }
    /**
     * For setting up a forwarder.
     * When the frontend is a ZMQ_XSUB socket, and the backend is a ZMQ_XPUB socket,
     * the proxy shall act as a message forwarder that collects messages from a
     * set of publishers and forwards these to a set of subscribers.
     * @return
     */
    @Bean
    public ZMQ.Socket proxyFrontendSocket() {
        ZMQ.Socket socket = proxyZContext().createSocket(SocketType.SUB);
        socket.bind("tcp://localhost:" + frontendPort);
        socket.subscribe(ZMQ.SUBSCRIPTION_ALL);
        return socket;
    }

    /**
     * For setting up a forwarder.
     * When the frontend is a ZMQ_XSUB socket, and the backend is a ZMQ_XPUB socket,
     * the proxy shall act as a message forwarder that collects messages from a
     * set of publishers and forwards these to a set of subscribers.
     * @return
     */
    @Bean
    public ZMQ.Socket proxyBackendSocket() {
        ZMQ.Socket socket = proxyZContext().createSocket(SocketType.PUB);
        socket.bind("tcp://localhost:" + backendPort);
        return socket;
    }

    /**
     * A socket for publishing message.
     */
    @Bean
    public ZMQ.Socket publishSocket() {
        ZMQ.Socket socket = proxyZContext().createSocket(SocketType.PUB);
        socket.connect("tcp://localhost:" + frontendPort);
        return socket;
    }

    /**
     * A socket for subscribing to specific topics.
     */
    @Bean
    public ZMQ.Socket subscriptionSocket() {
        ZMQ.Socket socket = proxyZContext().createSocket(SocketType.SUB);
        socket.connect("tcp://localhost:" + backendPort);
        //socket.subscribe(ZMQ.SUBSCRIPTION_ALL);
        socket.subscribe("server");
        return socket;
    }

    public void publish(String topic, String message) {
        publishSocket().send(topic + " " + message);
    }

    public String receive() {
        return subscriptionSocket().recvStr();
    }
}

