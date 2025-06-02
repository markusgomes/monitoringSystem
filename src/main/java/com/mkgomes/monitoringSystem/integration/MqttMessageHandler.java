package com.mkgomes.monitoringSystem.integration;

import org.springframework.stereotype.Component;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.SessaoRepository;
import com.mkgomes.monitoringSystem.service.IDhtControleService;
import com.mkgomes.monitoringSystem.service.IDhtService;
import com.mkgomes.monitoringSystem.util.SessaoContext;

@Component
public class MqttMessageHandler {

    private final IDhtService iDhtService;
    private final IDhtControleService iDhtControleService;
    private final SessaoRepository sessaoRepository;
    private final SessaoContext sessaoContext;

    public MqttMessageHandler(IDhtService iDhtService, IDhtControleService iDhtControleService,
            SessaoRepository sessaoRepository, SessaoContext sessaoContext) {
        this.iDhtService = iDhtService;
        this.iDhtControleService = iDhtControleService;
        this.sessaoRepository = sessaoRepository;
        this.sessaoContext = sessaoContext;
    }

    public void handle(String topic, String payload) {
        Long sessaoId = sessaoContext.getSessaoAtualId();

        try {
            SessaoEntity sessao = sessaoRepository.findById(sessaoId)
                    .orElseThrow(() -> new RuntimeException("Sessão não encontrada com ID: " + sessaoId));

            if (topic.equals("sensores/dht22")) {
                String[] values = payload.split(",");
                if (values.length == 2) {
                    DhtData dhtData = new DhtData();
                    dhtData.setTemperatura(Float.parseFloat(values[0]));
                    dhtData.setUmidade(Float.parseFloat(values[1]));

                    iDhtService.saveData(dhtData, sessao);
                }
            } else if (topic.equals("sensoresControle/dht22")) {
                String[] values = payload.split(",");
                if (values.length == 2) {
                    DhtData dhtData = new DhtData();
                    dhtData.setTemperatura(Float.parseFloat(values[0]));
                    dhtData.setUmidade(Float.parseFloat(values[1]));

                    iDhtControleService.saveData(dhtData, sessao);
                }
            }
            /*
             * } else if (topic.startsWith("sensores/max9814")) {
             * MaxData maxData = objectMapper.readValue(payload, MaxData.class);
             * 
             * } else if (topic.startsWith("sensoresControle/max9814")) {
             * 
             * }
             */
        } catch (Exception e) {
            System.err.println("Erro ao processar mensagem MQTT: " + e.getMessage());
        }
    }

}