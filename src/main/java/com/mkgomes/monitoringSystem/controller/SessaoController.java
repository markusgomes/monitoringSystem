package com.mkgomes.monitoringSystem.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.mkgomes.monitoringSystem.integration.CsvService;
import com.mkgomes.monitoringSystem.integration.MqttService;
import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;
import com.mkgomes.monitoringSystem.model.dto.SessaoRequest;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.model.entity.UsuarioEntity;
import com.mkgomes.monitoringSystem.repository.SessaoRepository;
import com.mkgomes.monitoringSystem.repository.UsuarioRepository;
import com.mkgomes.monitoringSystem.service.SessaoService;
import com.mkgomes.monitoringSystem.util.SessaoContext;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Map;
import java.util.Optional;

import org.springframework.core.io.Resource;
import org.springframework.core.io.UrlResource;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

@RestController
@RequestMapping("/")
public class SessaoController {

    private final SessaoRepository sessaoRepository;
    private final UsuarioRepository usuarioRepository;
    private final MqttService mqttService;
    private final CsvService csvService;
    private final SessaoService sessaoService;
    private final SessaoContext sessaoContext;

    public SessaoController(SessaoRepository sessaoRepository,
            UsuarioRepository usuarioRepository,
            MqttService mqttService, CsvService csvService,
            SessaoService sessaoService,
            SessaoContext sessaoContext) {
        this.sessaoRepository = sessaoRepository;
        this.usuarioRepository = usuarioRepository;
        this.mqttService = mqttService;
        this.csvService = csvService;
        this.sessaoService = sessaoService;
        this.sessaoContext = sessaoContext;

    }

    @PostMapping("/iniciar")
    public ResponseEntity<?> iniciarSessao(@RequestBody SessaoRequest request) {

        try {

            Optional<UsuarioEntity> usuarioOpt = usuarioRepository.findById(1L);
            if (usuarioOpt.isEmpty()) {
                return ResponseEntity.status(HttpStatus.NOT_FOUND)
                        .body(Map.of("erro", "Usuário não encontrado"));
            }

            if (request.getSensores() == null || request.getSensores().isEmpty()) {
                return ResponseEntity.badRequest()
                        .body(Map.of("erro", "Selecione pelo menos um sensor"));
            }

            SessaoEntity novaSessao = new SessaoEntity();
            novaSessao.setUsuario(usuarioOpt.get());
            novaSessao.setDuracao(request.getDuracao());
            novaSessao.setSensorDht(request.getSensores().contains("dht"));
            novaSessao.setSensorMax(request.getSensores().contains("max"));

            SessaoEntity sessaoSalva = sessaoRepository.save(novaSessao);

            sessaoContext.setSessaoAtualId(sessaoSalva.getId());

            StringBuilder sensoresAtivos = new StringBuilder();
            if (request.getSensores().contains("dht")) {
                sensoresAtivos.append("DHT");
                sensoresAtivos.append(",");
            }
            if (request.getSensores().contains("max")) {
                if (sensoresAtivos.length() > 0) {
                    sensoresAtivos.append(",");
                }
                sensoresAtivos.append("MAX");
            }

            String comando = String.format("START,%s", sensoresAtivos.toString());
            mqttService.publicarComando(comando);

            SessaoDTO sessaoDTO = sessaoService
                    .buscarSessaoComUsuarioDTO(sessaoSalva.getId());

            new java.util.Timer().schedule(new java.util.TimerTask() {
                @Override
                public void run() {
                    String comando = "STOP";
                    mqttService.publicarComando(comando);
                    System.out.println("Gerando CSV...");
                    try {
                        csvService.gerarCsvSessao(sessaoDTO);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }, request.getDuracao() * 60 * 1000L);

            return ResponseEntity.ok(Map.of(
                    "id", sessaoSalva.getId(),
                    "mensagem", "Sessão iniciada com sucesso"));

        } catch (Exception e) {
            return ResponseEntity.internalServerError()
                    .body(Map.of("erro", "Erro interno: " + e.getMessage()));
        }
    }

    @GetMapping("/{id}/csv")
    public ResponseEntity<?> downloadCsv(@PathVariable Long id) throws IOException {

        Path path = Paths.get("/tmp/dados_sessao_" + id + ".csv");
        if (!Files.exists(path)) {
            return ResponseEntity.status(HttpStatus.NOT_FOUND).build();
        }

        Optional<SessaoEntity> sessaoOpt = sessaoRepository.findById(id);
        if (sessaoOpt.isEmpty()) {
            return ResponseEntity.status(HttpStatus.NOT_FOUND)
                    .body("Sessão não encontrada");
        }
        Resource resource = new UrlResource(path.toUri());

        return ResponseEntity.ok()
                .header("Content-Disposition", "attachment; filename=\"dados_sessao_" + id + ".csv\"")
                .contentType(MediaType.parseMediaType("text/csv"))
                .body(resource);
    }

}
