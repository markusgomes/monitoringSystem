package com.mkgomes.monitoringSystem.integration;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.CicloDTO;
import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;
import com.mkgomes.monitoringSystem.model.entity.DhtControleEntity;
import com.mkgomes.monitoringSystem.model.entity.DhtTesteEntity;
import com.mkgomes.monitoringSystem.model.entity.MlxControleEntity;
import com.mkgomes.monitoringSystem.model.entity.MlxTesteEntity;
import com.mkgomes.monitoringSystem.repository.DhtControleRepository;
import com.mkgomes.monitoringSystem.repository.DhtTesteRepository;
import com.mkgomes.monitoringSystem.repository.MaxControleRepository;
import com.mkgomes.monitoringSystem.repository.MaxTesteRepository;
import com.mkgomes.monitoringSystem.repository.MlxControleRepository;
import com.mkgomes.monitoringSystem.repository.MlxTesteRepository;
import com.mkgomes.monitoringSystem.util.SensorType;

@Service
public class CsvService {

    private final DhtTesteRepository dhtRepository;
    private final DhtControleRepository dhtControleRepository;
    private final MlxTesteRepository mlxRepository;
    private final MlxControleRepository mlxControleRepository;
    private final MaxTesteRepository maxRepository;
    private final MaxControleRepository maxControleRepository;

    public CsvService(DhtTesteRepository dhtRepository, DhtControleRepository dhtControleRepository,
            MlxTesteRepository mlxRepository, MlxControleRepository mlxControleRepository,
            MaxTesteRepository maxRepository, MaxControleRepository maxControleRepository) {
        this.dhtRepository = dhtRepository;
        this.dhtControleRepository = dhtControleRepository;
        this.mlxRepository = mlxRepository;
        this.mlxControleRepository = mlxControleRepository;
        this.maxRepository = maxRepository;
        this.maxControleRepository = maxControleRepository;

    }

    @Async
    public void gerarCsvSessaoAsync(SessaoDTO sessaoDTO, List<CicloDTO> cicloDTO) {
        try {
            gerarCsvSessao(sessaoDTO, cicloDTO);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void gerarCsvSessao(SessaoDTO sessaoDTO, List<CicloDTO> cicloDTO) throws IOException {

        StringBuilder csv = new StringBuilder();
        csv.append("Usuário:;").append(sessaoDTO.getUsuario().getNome()).append("\n");
        csv.append("Início da Sessão:;").append(sessaoDTO.getDataHoraCriacao()).append("\n");

        String sensoresAtivos = "";
        if (sessaoDTO.isSensorDht())
            sensoresAtivos += " - DHT22";
        if (sessaoDTO.isSensorMlx())
            sensoresAtivos += " - MLX90614";
        if (sessaoDTO.isSensorMax())
            sensoresAtivos += " - MAX9814";

        csv.append("Sensor(es):;").append(sensoresAtivos).append("\n");
        csv.append("Amostra:;").append(sessaoDTO.getAmostra()).append("\n");
        csv.append("Descrição:;").append(sessaoDTO.getDescricao()).append("\n\n");

        List<DhtTesteEntity> dhtDados = dhtRepository.findBySessaoId(sessaoDTO.getId());
        List<DhtControleEntity> dhtControleDados = dhtControleRepository.findBySessaoId(sessaoDTO.getId());
        List<MlxTesteEntity> mlxDados = mlxRepository.findBySessaoId(sessaoDTO.getId());
        List<MlxControleEntity> mlxControleDados = mlxControleRepository.findBySessaoId(sessaoDTO.getId());

        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");

        for (int cicloIdx = 0; cicloIdx < cicloDTO.size(); cicloIdx++) {
            CicloDTO ciclo = cicloDTO.get(cicloIdx);

            csv.append("Ciclo: ").append(cicloIdx + 1).append("\n");
            csv.append("Temperatura (ºC):;")
                    .append(String.format("%.1f", ciclo.getTemperatura()).replace('.', ','))
                    .append("\n");
            csv.append("Duração (min):;").append(ciclo.getDuracao()).append("\n\n\n");

            List<String> headers = new ArrayList<>();
            headers.add("hora");
            headers.add("cronometro");

            if (sessaoDTO.isSensorDht())
                headers.addAll(SensorType.DHT.getColunas());
            if (sessaoDTO.isSensorMlx())
                headers.addAll(SensorType.MLX.getColunas());
            if (sessaoDTO.isSensorMax())
                headers.addAll(SensorType.MAX.getColunas());

            csv.append(String.join(";", headers)).append("\n");

            int tamanho = obterMaiorTamanhoDados(sessaoDTO, dhtDados, dhtControleDados, mlxDados, mlxControleDados);
            LocalDateTime startTime = null;

            for (int i = 0; i < tamanho; i++) {
                StringBuilder linha = new StringBuilder();

                String hora = "--:--:--";
                String cronometro = "--:--";
                LocalDateTime currentDateTime = null;

                if (sessaoDTO.isSensorDht() && i < dhtDados.size() && dhtDados.get(i) != null) {
                    currentDateTime = dhtDados.get(i).getDataHora();
                } /*
                   * else if (sessaoDTO.isSensorMax() && i < mlxDados.size() && mlxDados.get(i) !=
                   * null) {
                   * currentDateTime = mlxDados.get(i).getDataHora();
                   * }
                   */

                if (currentDateTime != null) {
                    hora = sdf.format(Date.from(currentDateTime.atZone(ZoneId.systemDefault()).toInstant()));
                    
                    if (startTime == null) {
                        startTime = currentDateTime;
                        cronometro = "00:00";
                    } else {
                        long segundos = java.time.Duration.between(startTime, currentDateTime).getSeconds();
                        long minutos = segundos / 60;
                        long restoSegundos = segundos % 60;
                        cronometro = String.format("%02d:%02d", minutos, restoSegundos);
                    }
                }

                linha.append(hora).append(";").append(cronometro).append(";");
                
                if (sessaoDTO.isSensorDht()) {
                    DhtTesteEntity teste = (i < dhtDados.size()) ? dhtDados.get(i) : null;
                    DhtControleEntity controle = (i < dhtControleDados.size()) ? dhtControleDados.get(i) : null;

                    if (teste != null && controle != null) {
                        String temperaturaControle = String.format("%.1f", controle.getTemperatura()).replace('.', ',');
                        String temperaturaTest = String.format("%.1f", teste.getTemperatura()).replace('.', ',');
                        String deltaTemperatura = String
                                .format("%.1f", controle.getTemperatura() - teste.getTemperatura())
                                .replace('.', ',');

                        String umidadeControle = String.format("%.1f", controle.getUmidade()).replace('.', ',');
                        String umidadeTest = String.format("%.1f", teste.getUmidade()).replace('.', ',');
                        String deltaUmidade = String.format("%.1f", controle.getUmidade() - teste.getUmidade()).replace(
                                '.',
                                ',');

                        linha.append(temperaturaControle).append(";")
                                .append(temperaturaTest).append(";")
                                .append(deltaTemperatura).append(";")
                                .append(umidadeControle).append(";")
                                .append(umidadeTest).append(";")
                                .append(deltaUmidade).append(";");
                    } else {
                        linha.append("--;--;--;--;--;--;");
                    }
                }

                csv.append(linha).append("\n");
            }

            csv.append("\n\n");
        }

        String dataHoraAtual = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss").format(new Date());

        Path path = Paths
                .get("/tmp/dados_sessao_"
                        + sessaoDTO.getId() + "_"
                        + dataHoraAtual + "_"
                        + sessaoDTO.getAmostra()
                        + ".csv");
        Files.writeString(path, csv.toString(), StandardCharsets.UTF_8);
    }

    private int obterMaiorTamanhoDados(SessaoDTO sessaoDTO,
            List<DhtTesteEntity> dht, List<DhtControleEntity> dhtCtrl,
            List<MlxTesteEntity> mlx, List<MlxControleEntity> mlxCtrl) {

        int max = 0;
        if (sessaoDTO.isSensorDht()) {
            max = Math.max(max, Math.max(dht.size(), dhtCtrl.size()));
        }
        if (sessaoDTO.isSensorMlx()) {
            max = Math.max(max, Math.max(mlx.size(), mlxCtrl.size()));
        }
        return max;
    }
}