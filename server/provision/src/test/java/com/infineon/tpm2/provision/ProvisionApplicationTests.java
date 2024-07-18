package com.infineon.tpm2.provision;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.web.server.LocalServerPort;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.reactive.server.WebTestClient;

import static com.infineon.tpm2.provision.Constant.WEBAPI_V1_PING;

@ActiveProfiles("test")
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
class ProvisionApplicationTests {
    @LocalServerPort
    private int serverPort;
    @Autowired
    private WebTestClient webTestClient;

    @Test
    void cors() {
        /* "Access-Control-Allow-Origin" is not set if "Origin" is not set */
        String result = webTestClient
                .get().uri("http://localhost:" + serverPort + WEBAPI_V1_PING)
                .exchange()
                .expectStatus().isOk()
                .expectHeader().contentType("text/plain;charset=UTF-8")
                .expectHeader().doesNotExist("Access-Control-Allow-Origin")
                .expectBody(String.class)
                .returnResult()
                .getResponseBody();
        Assertions.assertEquals(result, "pong");

        /* "Access-Control-Allow-Origin" is set if "Origin" is set */
        result = webTestClient
                .get().uri("http://localhost:" + serverPort + WEBAPI_V1_PING)
                .header("Origin", "http://somewhere")
                .exchange()
                .expectStatus().isOk()
                .expectHeader().contentType("text/plain;charset=UTF-8")
                .expectHeader().valueEquals("Access-Control-Allow-Origin", "*")
                .expectBody(String.class)
                .returnResult()
                .getResponseBody();
        Assertions.assertEquals(result, "pong");

        /* "Access-Control-Allow-Origin" is not set if "Origin" is set to server URI */
        result = webTestClient
                .get().uri("http://localhost:" + serverPort + WEBAPI_V1_PING)
                .header("Origin", "http://localhost:" + serverPort)
                .exchange()
                .expectStatus().isOk()
                .expectHeader().contentType("text/plain;charset=UTF-8")
                .expectHeader().doesNotExist("Access-Control-Allow-Origin")
                .expectBody(String.class)
                .returnResult()
                .getResponseBody();
        Assertions.assertEquals(result, "pong");
    }

    @Test
    void v1Ping() {
        String result = webTestClient
                .get().uri("http://localhost:" + serverPort + WEBAPI_V1_PING)
                .exchange()
                .expectStatus().isOk()
                .expectHeader().contentType("text/plain;charset=UTF-8")
                .expectHeader().doesNotExist("Access-Control-Allow-Origin")
                .expectBody(String.class)
                .returnResult()
                .getResponseBody();
        Assertions.assertEquals(result, "pong");
    }

}
