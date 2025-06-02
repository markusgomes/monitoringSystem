package com.mkgomes.monitoringSystem.integration;

import java.util.Arrays;
import java.util.List;

import org.eclipse.paho.mqttv5.client.IMqttToken;
import org.eclipse.paho.mqttv5.client.MqttAsyncClient;
import org.eclipse.paho.mqttv5.client.MqttCallback;
import org.eclipse.paho.mqttv5.client.MqttConnectionOptions;
import org.eclipse.paho.mqttv5.client.MqttDisconnectResponse;
import org.eclipse.paho.mqttv5.common.MqttException;
import org.eclipse.paho.mqttv5.common.MqttMessage;
import org.eclipse.paho.mqttv5.common.MqttSecurityException;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class MqttConfig {

    private final MqttMessageHandler mqttMessageHandler;

    private final MqttProperties mqttProps;

    public MqttConfig(MqttProperties mqttProps, MqttMessageHandler mqttMessageHandler) {
        this.mqttProps = mqttProps;
        this.mqttMessageHandler = mqttMessageHandler;
    }

    @Bean
    public MqttConnectionOptions mqttConnectionOptions() {
        MqttConnectionOptions mqttConnectionOptions = new MqttConnectionOptions();

        mqttConnectionOptions.setUserName(mqttProps.getUser());
        mqttConnectionOptions.setPassword(mqttProps.getPasswordBytes());

        mqttConnectionOptions.setAutomaticReconnect(true);
        mqttConnectionOptions.setCleanStart(true);
        mqttConnectionOptions.setConnectionTimeout(15);
        mqttConnectionOptions.setKeepAliveInterval(60);

        return mqttConnectionOptions;
    }

    @Bean
    public MqttAsyncClient mqttClient(MqttConnectionOptions mqttConnectionOptions) throws MqttException {

        MqttAsyncClient client = new MqttAsyncClient(
                mqttProps.getBroker(),
                mqttProps.getClientId());

        client.setCallback(new MqttCallback() {
            @Override
            public void messageArrived(String topic, MqttMessage message) {
                String payload = new String(message.getPayload());
                mqttMessageHandler.handle(topic, payload);
            }

            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                System.out.println((reconnect ? "Reconectado" : "Conectado") + " ao broker MQTT: " + serverURI);
            }

            @Override
            public void disconnected(MqttDisconnectResponse disconnectResponse) {
                System.out.println("Desconectado do broker MQTT: " + disconnectResponse.getReasonString());
            }

            @Override
            public void mqttErrorOccurred(MqttException exception) {
                System.err.println("Erro MQTT: " + exception.getMessage());
            }

            @Override
            public void deliveryComplete(IMqttToken token) {
            }

            @Override
            public void authPacketArrived(int reasonCode,
                    org.eclipse.paho.mqttv5.common.packet.MqttProperties properties) {
            }

        });

        connectAndSubscribe(client, mqttConnectionOptions);
        return client;
    }

    private void connectAndSubscribe(MqttAsyncClient client, MqttConnectionOptions mqttConnectionOptions) {
        try {
            if (!client.isConnected()) {
                IMqttToken token = client.connect(mqttConnectionOptions);
                token.waitForCompletion();
                System.out.println("Conectado ao broker MQTT: " + mqttProps.getBroker());
                subscribeToTopics(client);
            }
        } catch (MqttSecurityException e) {
            System.err.println("Erro de autenticação: " + e.getMessage());
            throw new RuntimeException("Falha na autenticação MQTT", e);
        } catch (MqttException e) {
            System.err.println("Erro de conexão: " + e.getMessage());
            throw new RuntimeException("Falha na conexão MQTT", e);
        }
    }

    private void subscribeToTopics(MqttAsyncClient client) {
        String[] rawTopics = mqttProps.getTopics().split(",");
        List<String> topics = Arrays.stream(rawTopics).map(String::trim).filter(t -> !t.isEmpty()).toList();

        if (topics.isEmpty()) {
            System.out.println("Nenhum tópico válido configurado para inscrição");
            return;
        }

        int qos = 1;
        int[] qoss = new int[topics.size()];
        Arrays.fill(qoss, qos);

        String[] topicsArray = topics.toArray(new String[0]);

        try {
            client.subscribe(topicsArray, qoss);
            System.out.printf("Inscrito em tópicos %s com QoS %d%n", Arrays.toString(topicsArray), qos);
        } catch (MqttException e) {
            System.out.printf("Erro ao inscrever tópicos %s: %s%n", Arrays.toString(topicsArray), e.getMessage());
        }
    }

}
