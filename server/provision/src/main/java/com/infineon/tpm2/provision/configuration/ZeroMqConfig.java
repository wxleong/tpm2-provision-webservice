package com.infineon.tpm2.provision.configuration;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.channel.PublishSubscribeChannel;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.zeromq.dsl.ZeroMq;
import org.springframework.messaging.MessageChannel;
import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

@Configuration
public class ZeroMqConfig {

    @Bean
    ZContext zContext() {
        ZContext context = new ZContext();
        return context;
    }

    @Bean
    public MessageChannel subscribeChannel() {
        return new PublishSubscribeChannel();
    }

    /**
     * Redirect the message flow from ZeroMqChannel to subscribeChannel.
     * Use @ServiceActivator to register the handler for receiving messages.
     * @return
     */
    @Bean
    public IntegrationFlow inFlow() {
        return IntegrationFlow
                .from(ZeroMq
                        .inboundChannelAdapter(zContext(), SocketType.SUB)
                        .connectUrl("tcp://127.0.0.1:5555")
                        //.consumeDelay(Duration.ofMillis(100))
                        .receiveRaw(true))
                .channel(subscribeChannel())
                /*.transform(Transformers.objectToString())
                  .handle(message -> {
                    System.out.println("Received message: " + message);
                })*/
                .get();
    }

    @Bean
    ZMQ.Socket publisherSocket() {
        ZMQ.Socket pubSock = zContext().createSocket(SocketType.PUB);
        pubSock.bind("tcp://127.0.0.1:5555");
        return pubSock;
    }

    public void publish(String topic, String message) {
        publisherSocket().send(topic + " " + message);
    }
}
