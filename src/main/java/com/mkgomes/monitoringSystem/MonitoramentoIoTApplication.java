package com.mkgomes.monitoringSystem;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties
public class MonitoramentoIoTApplication {

	public static void main(String[] args) {
		SpringApplication.run(MonitoramentoIoTApplication.class, args);
	}

}
