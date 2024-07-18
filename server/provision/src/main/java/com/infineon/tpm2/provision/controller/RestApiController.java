package com.infineon.tpm2.provision.controller;

import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import static com.infineon.tpm2.provision.Constant.WEBAPI_V1_PING;

/*
 * The "Access-Control-Allow-Origin" response header will only be set if
 * the request header includes "Origin" and the value of "Origin" is
 * different from the server URI.
 */
@CrossOrigin(origins = "*")
@RestController
public class RestApiController {

    @GetMapping(value = WEBAPI_V1_PING, produces = "text/plain;charset=UTF-8")
    public String ping() {
        return "pong";
    }

}
