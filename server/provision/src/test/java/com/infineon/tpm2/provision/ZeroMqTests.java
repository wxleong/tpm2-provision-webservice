package com.infineon.tpm2.provision;

import com.infineon.tpm2.provision.configuration.ZeroMqConfig;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.dsl.Transformers;
import org.springframework.integration.dsl.context.IntegrationFlowContext;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.test.context.ActiveProfiles;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

@ActiveProfiles("test")
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
public class ZeroMqTests {
    @Autowired
    private ZeroMqConfig zeroMqConfig;
    @Autowired
    private MessageChannel subscribeChannel;
    @Autowired
    IntegrationFlowContext integrationFlowContext;

    @Test
    void publishSubscribe() throws InterruptedException {
        BlockingQueue<Message<?>> messages = new LinkedBlockingQueue<>();
        String topic = "topic";
        String message = "Broadcast from Spring Integration";

        IntegrationFlow consumerFlow =
            IntegrationFlow
                .from(subscribeChannel)
                .transform(Transformers.objectToString())
                .handle(messages::offer)
                .get();
        integrationFlowContext.registration(consumerFlow).register();

        zeroMqConfig.publish(topic, message);
        Message<?> receivedMessage = messages.poll(10, TimeUnit.SECONDS);
        Assertions.assertEquals("[ " + topic + " " + message + " ]", receivedMessage.getPayload());

        integrationFlowContext
            .getRegistry()
            .values()
            .forEach(IntegrationFlowContext.IntegrationFlowRegistration::destroy);
    }
}
