package com.infineon.tpm2.provision.configuration;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.config.EnableIntegration;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.zeromq.dsl.ZeroMq;
import org.springframework.messaging.MessageChannel;
import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

@Configuration
@EnableIntegration
public class ZeroMqConfig {

    public static final String TOPIC_TPM2_RESPONSE = "tpm2resp:";
    public static final String TOPIC_TPM2_COMMAND = "tpm2cmd:";

    @Bean
    public MessageChannel subscribeChannel() {
        return new DirectChannel();
    }

    @Bean
    ZContext zContext() {
        ZContext context = new ZContext();
        return context;
    }

    /**
     * Redirect message flow from ZeroMqChannel to subscribeChannel
     * @param context
     * @return
     */
    @Bean
    public IntegrationFlow inFlow(ZContext context) {
        return IntegrationFlow
                .from(ZeroMq
                        .inboundChannelAdapter(context, SocketType.SUB)
                        .connectUrl("tcp://127.0.0.1:5555")
                        .topics(TOPIC_TPM2_COMMAND, TOPIC_TPM2_RESPONSE)
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
    ZMQ.Socket publisherSocket(ZContext zContext) {
        ZMQ.Socket pubSock = zContext.createSocket(SocketType.PUB);
        pubSock.bind("tcp://127.0.0.1:5555");
        return pubSock;
    }

    public static void publish(ZMQ.Socket sock, String message) {
        sock.send(TOPIC_TPM2_RESPONSE + " " + message);
    }
}
