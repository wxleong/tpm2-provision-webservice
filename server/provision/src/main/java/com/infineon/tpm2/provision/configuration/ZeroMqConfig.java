package com.infineon.tpm2.provision.configuration;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.support.MessageBuilder;
import org.springframework.integration.zeromq.ZeroMqProxy;
import org.springframework.integration.zeromq.channel.ZeroMqChannel;
import org.zeromq.ZContext;

import java.time.Duration;

@Configuration
public class ZeroMqConfig {

    @Value("${zeromq.publisher.port}")
    private int frontendPort;

    @Value("${zeromq.subscriber.port}")
    private int backendPort;

    @Bean
    ZContext zContext() {
        ZContext context = new ZContext();
        return context;
    }

    @Bean
    ZeroMqProxy zeroMqProxy() {
        ZeroMqProxy proxy = new ZeroMqProxy(zContext(), ZeroMqProxy.Type.SUB_PUB);
        proxy.setExposeCaptureSocket(true);
        proxy.setFrontendPort(frontendPort);
        proxy.setBackendPort(backendPort);
        return proxy;
    }
    @Bean
    ZeroMqChannel zeroMqPubSubChannel(ZContext context) {
        ZeroMqChannel channel = new ZeroMqChannel(context, true);
        channel.setConnectUrl("tcp://localhost:" + frontendPort + ":" + backendPort);
        channel.setConsumeDelay(Duration.ofMillis(100));
        return channel;
    }

    public void publish(String topic, String message) {
        zeroMqPubSubChannel(zContext()).send(MessageBuilder.withPayload(topic + " " + message).setHeader("topic", topic).build());
    }
}
