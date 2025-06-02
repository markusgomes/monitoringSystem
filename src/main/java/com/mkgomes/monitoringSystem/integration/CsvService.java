package com.mkgomes.monitoringSystem.integration;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.Date;
import java.util.List;

import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;
import com.mkgomes.monitoringSystem.model.entity.DhtControleEntity;
import com.mkgomes.monitoringSystem.model.entity.DhtEntity;
import com.mkgomes.monitoringSystem.repository.DhtControleRepository;
import com.mkgomes.monitoringSystem.repository.DhtRepository;


@Service
public class CsvService {

    private final DhtRepository dhtRepository;
    private final DhtControleRepository dhtControleRepository;

    public CsvService(DhtRepository dhtRepository,
            DhtControleRepository dhtControleRepository) {
        this.dhtRepository = dhtRepository;
        this.dhtControleRepository = dhtControleRepository;
    }

    @Async
    public void gerarCsvSessaoAsync(SessaoDTO sessaoDTO) {
        try {
            gerarCsvSessao(sessaoDTO);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void gerarCsvSessao(SessaoDTO sessaoDTO) throws IOException {

        StringBuilder csv = new StringBuilder();
        csv.append("Usuário,").append(sessaoDTO.getUsuario().getNome()).append("\n");
        csv.append("Início da Sessão,").append(sessaoDTO.getDataHoraCriacao()).append("\n");
        csv.append("Duração (min),").append(sessaoDTO.getDuracao()).append("\n\n\n");

        csv.append("timestamp,temperatura_test,temperatura_controle,umidade_test, umidade_controle\n");

        List<DhtEntity> dhtDados = dhtRepository.findBySessaoId(sessaoDTO.getId());
        List<DhtControleEntity> dhtControleDados = dhtControleRepository.findBySessaoId(sessaoDTO.getId());

        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");

        int tamanho = Math.max(dhtDados.size(), dhtControleDados.size());
        for (int i = 0; i < tamanho; i++) {
            DhtEntity teste = dhtDados.get(i);
            DhtControleEntity controle = dhtControleDados.get(i);

            LocalDateTime ldt = teste.getDataHora();
            Date dataComoDate = Date.from(ldt.atZone(ZoneId.systemDefault()).toInstant());
            String timestamp = sdf.format(dataComoDate);
            
            String temperaturaTest = String.valueOf(teste.getTemperatura());
            String temperaturaControle = String.valueOf(controle.getTemperatura());

            String umidadeTest = String.valueOf(teste.getUmidade());
            String umidadeControle = String.valueOf(controle.getUmidade());

            csv.append(timestamp).append(",")
            .append(temperaturaTest).append(",")
            .append(temperaturaControle).append(",")
            .append(umidadeTest).append(",")
            .append(umidadeControle).append("\n");
        }

        Path path = Paths.get("/tmp/dados_sessao_" + sessaoDTO.getId() + ".csv");
        Files.writeString(path, csv.toString(), StandardCharsets.UTF_8);
    }
}