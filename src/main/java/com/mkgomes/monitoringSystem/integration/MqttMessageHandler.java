package com.mkgomes.monitoringSystem.integration;

import org.springframework.stereotype.Component;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.dto.MlxData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.CicloRepository;
import com.mkgomes.monitoringSystem.repository.SessaoRepository;
import com.mkgomes.monitoringSystem.service.IDhtControleService;
import com.mkgomes.monitoringSystem.service.IDhtTesteService;
import com.mkgomes.monitoringSystem.service.IMlxControleService;
import com.mkgomes.monitoringSystem.service.IMlxTesteService;
import com.mkgomes.monitoringSystem.util.SessaoContext;

@Component
public class MqttMessageHandler {

    private final SessaoRepository sessaoRepository;
    private final SessaoContext sessaoContext;
    private final CicloRepository cicloRepository;

    private final IDhtTesteService iDhtTesteService;
    private final IDhtControleService iDhtControleService;
    private final IMlxTesteService iMlxTesteService;
    private final IMlxControleService iMlxControleService;


    public MqttMessageHandler(IDhtTesteService iDhtTesteService, IDhtControleService iDhtControleService,
                            IMlxTesteService iMlxTesteService, IMlxControleService iMlxControleService,
                            SessaoRepository sessaoRepository, CicloRepository cicloRepository, SessaoContext sessaoContext) {
        this.iDhtTesteService = iDhtTesteService;
        this.iDhtControleService = iDhtControleService;
        this.iMlxTesteService = iMlxTesteService;
        this.iMlxControleService = iMlxControleService;

        this.sessaoRepository = sessaoRepository;
        this.sessaoContext = sessaoContext;
        this.cicloRepository = cicloRepository;
    }

    public void handle(String topic, String payload) {
        Long sessaoId = sessaoContext.getSessaoAtualId();
        CicloEntity ciclo = cicloRepository
                .findTopBySessaoIdOrderByDataHoraCriacaoDesc(sessaoId)
                .orElseThrow(() -> new RuntimeException("Ciclo não encontrado para a sessão ID: " + sessaoId));

        try {
            SessaoEntity sessao = sessaoRepository.findById(sessaoId)
                    .orElseThrow(() -> new RuntimeException("Sessão não encontrada com ID: " + sessaoId));

            boolean isDhtTeste = topic.equals("sensoresDht/teste");
            boolean isDhtControle = topic.equals("sensoresDht/controle");
            boolean isMlxTeste = topic.equals("sensoresMlx/teste");
            boolean isMlxControle = topic.equals("sensoresMlx/controle");

            if (isDhtTeste || isDhtControle) {

                String[] linhas = payload.split("\n");

                for (String linha : linhas) {
                    String[] partes = linha.trim().split(",");
                    if (partes.length < 3)
                        continue;

                    try {
                        long millisRelativo = Long.parseLong(partes[0].trim());
                        float temperatura = Float.parseFloat(partes[1].trim());
                        float umidade = Float.parseFloat(partes[2].trim());

                        DhtData dhtData = new DhtData();
                        dhtData.setMillisRelativo(millisRelativo);
                        dhtData.setTemperatura(temperatura);
                        dhtData.setUmidade(umidade);

                        if (isDhtTeste) {
                            iDhtTesteService.saveData(dhtData, sessao, ciclo);
                        } else {
                            iDhtControleService.saveData(dhtData, sessao, ciclo);
                        }

                    } catch (Exception e) {
                        System.err.println("Erro ao processar linha CSV DHT: " + linha + " → " + e.getMessage());
                    }
                }
            }

            if (isMlxTeste || isMlxControle) {

                String[] linhas = payload.split("\n");

                for (String linha : linhas) {
                    String[] partes = linha.trim().split(",");
                    if (partes.length < 3)
                        continue;

                    try {
                        long millisRelativo = Long.parseLong(partes[0].trim());
                        float tempAmb = Float.parseFloat(partes[1].trim());
                        float TempIR = Float.parseFloat(partes[2].trim());

                        MlxData mlxData = new MlxData();
                        mlxData.setMillisRelativo(millisRelativo);
                        mlxData.setTemp_a(tempAmb);
                        mlxData.setTemp_ir(TempIR);

                        if (isMlxTeste) {
                            iMlxTesteService.saveData(mlxData, sessao, ciclo);
                        } else {
                            iMlxControleService.saveData(mlxData, sessao, ciclo);
                        }

                    } catch (Exception e) {
                        System.err.println("Erro ao processar linha CSV MLX: " + linha + " → " + e.getMessage());
                    }
                }
            }

        } catch (Exception e) {
            System.err.println("Erro ao processar mensagem MQTT: " + e.getMessage());
        }
    }
}