package com.infineon.tpm2.provision;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

@EnableScheduling
@SpringBootApplication
public class ProvisionApplication {

	public static void main(String[] args) {
		SpringApplication.run(ProvisionApplication.class, args);
	}

}
