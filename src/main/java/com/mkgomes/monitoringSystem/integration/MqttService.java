package com.mkgomes.monitoringSystem.integration;

import org.eclipse.paho.mqttv5.client.MqttAsyncClient;
import org.eclipse.paho.mqttv5.common.MqttException;
import org.eclipse.paho.mqttv5.common.MqttMessage;
import org.springframework.stereotype.Service;

@Service
public class MqttService {

    private final MqttAsyncClient mqttClient;

    public MqttService(MqttAsyncClient mqttClient) {
        this.mqttClient = mqttClient;
    }

    public void publicarComando(String comando) {
        String topico = "sensores/control";

        try {
            MqttMessage message = new MqttMessage(comando.getBytes());
            message.setQos(1);
            mqttClient.publish(topico, message);

        } catch (MqttException e) {
            System.err.println("Erro ao publicar comando MQTT: " + e.getMessage());
        }
    }

}
