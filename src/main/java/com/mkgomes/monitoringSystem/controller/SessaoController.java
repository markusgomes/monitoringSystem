package com.mkgomes.monitoringSystem.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.mkgomes.monitoringSystem.integration.CsvService;
import com.mkgomes.monitoringSystem.integration.MqttService;
import com.mkgomes.monitoringSystem.mapper.CicloMapper;
import com.mkgomes.monitoringSystem.model.dto.CicloDTO;
import com.mkgomes.monitoringSystem.model.dto.CicloRequest;
import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;
import com.mkgomes.monitoringSystem.model.dto.SessaoRequest;
import com.mkgomes.monitoringSystem.model.dto.StartSessaoRequest;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.model.entity.UsuarioEntity;
import com.mkgomes.monitoringSystem.repository.CicloRepository;
import com.mkgomes.monitoringSystem.repository.SessaoRepository;
import com.mkgomes.monitoringSystem.repository.UsuarioRepository;
import com.mkgomes.monitoringSystem.service.CicloService;
import com.mkgomes.monitoringSystem.service.ICicloService;
import com.mkgomes.monitoringSystem.service.ISessaoService;
import com.mkgomes.monitoringSystem.service.SessaoService;
import com.mkgomes.monitoringSystem.util.SessaoContext;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

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
    private final CicloRepository cicloRepository;
    private final UsuarioRepository usuarioRepository;

    private final ISessaoService iSessaoService;
    private final ICicloService iCicloService;

    private final MqttService mqttService;
    private final CsvService csvService;
    private final SessaoContext sessaoContext;

    public SessaoController(SessaoRepository sessaoRepository, CicloRepository cicloRepository,
            UsuarioRepository usuarioRepository,
            MqttService mqttService, CsvService csvService, SessaoContext sessaoContext,
            ISessaoService iSessaoService, ICicloService iCicloService) {
        this.sessaoRepository = sessaoRepository;
        this.cicloRepository = cicloRepository;
        this.usuarioRepository = usuarioRepository;
        this.mqttService = mqttService;
        this.csvService = csvService;
        this.iSessaoService = iSessaoService;
        this.iCicloService = iCicloService;
        this.sessaoContext = sessaoContext;
    }

    @PostMapping("/iniciar")
    public ResponseEntity<?> iniciarSessao(@RequestBody StartSessaoRequest request) {

        try {

            Optional<UsuarioEntity> usuarioOpt = usuarioRepository.findById(1L);
            if (usuarioOpt.isEmpty()) {
                return ResponseEntity.status(HttpStatus.NOT_FOUND)
                        .body(Map.of("erro", "Usuário não encontrado"));
            }

            List<String> sensoresSelecionados = request.getSessaoRequest().getSensores();
            if (sensoresSelecionados == null || sensoresSelecionados.isEmpty()) {
                return ResponseEntity.badRequest()
                        .body(Map.of("erro", "Selecione pelo menos um sensor"));
            }

            SessaoEntity novaSessao = new SessaoEntity();
            novaSessao.setUsuario(usuarioOpt.get());
            novaSessao.setAmostra(request.getSessaoRequest().getAmostra());
            novaSessao.setDescricao(request.getSessaoRequest().getDescricao());
            novaSessao.setSensorDht(request.getSessaoRequest().getSensores().contains("dht"));
            novaSessao.setSensorMlx(request.getSessaoRequest().getSensores().contains("mlx"));
            novaSessao.setSensorMax(request.getSessaoRequest().getSensores().contains("max"));
            SessaoEntity sessaoSalva = sessaoRepository.save(novaSessao);

            CicloEntity ciclo = new CicloEntity();
            ciclo.setSessao(sessaoSalva);
            ciclo.setDuracao(request.getCicloRequest().getDuracao());
            ciclo.setTemperatura(request.getCicloRequest().getTemperatura());
            ciclo.setQuant_cap(10);
            CicloEntity cicloSalvo = iCicloService.saveData(ciclo);

            List<CicloDTO> ciclosDTO = List.of(CicloMapper.toDTO(cicloSalvo));

            sessaoContext.setSessaoAtualId(sessaoSalva.getId());

            String comandoStart = sensoresSelecionados
                    .stream()
                    .map(String::toUpperCase)
                    .map(String::trim)
                    .filter(s -> !s.isBlank())
                    .distinct()
                    .collect(Collectors.joining(","));
            mqttService.publicarComando("START," + comandoStart + "," + 10);

            SessaoDTO sessaoDTO = iSessaoService
                    .buscarSessaoComUsuarioDTO(sessaoSalva.getId());

            ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

            scheduler.schedule(() -> {
                try {
                    mqttService.publicarComando("STOP");
                    System.out.println("Gerando CSV...");
                    /*csvService.gerarCsvSessao(sessaoDTO, ciclosDTO);*/
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    scheduler.shutdown(); // ✅ importante: libera thread após execução
                }
            }, cicloSalvo.getDuracao(), TimeUnit.MINUTES);

            return ResponseEntity.ok(Map.of(
                    "id", sessaoSalva.getId(),
                    "mensagem", "Sessão iniciada com sucesso"));

        } catch (Exception e) {
            return ResponseEntity.internalServerError()
                    .body(Map.of("erro", "Erro interno: " + e.getMessage()));
        }
    }

    @GetMapping("/{id}/csv")
    public ResponseEntity<?> downloadCsv(@PathVariable Long id) {
        try {
            Optional<SessaoEntity> sessaoOpt = sessaoRepository.findById(id);
            if (sessaoOpt.isEmpty()) {
                return ResponseEntity.status(HttpStatus.NOT_FOUND)
                        .body("Sessão não encontrada");
            }

            String prefixo = "dados_sessao_" + id + "_";
            Path dir = Paths.get("/tmp");

            // Procura por arquivos existentes
            Optional<Path> match;
            try (var files = Files.list(dir)) {
                match = files
                        .filter(f -> f.getFileName().toString().startsWith(prefixo)
                                && f.getFileName().toString().endsWith(".csv"))
                        .findFirst();
            }

            Path csvPath;

            if (match.isPresent()) {
                csvPath = match.get();
            } else {
                // Gera um novo CSV
                SessaoDTO sessaoDTO = iSessaoService.buscarSessaoComUsuarioDTO(id);
                List<CicloDTO> ciclos = iCicloService.findCiclosBySessaoId(id);
                csvService.gerarCsvSessao(sessaoDTO, ciclos);

                // Após gerar, localizar o novo arquivo
                try (var files = Files.list(dir)) {
                    match = files
                            .filter(f -> f.getFileName().toString().startsWith(prefixo)
                                    && f.getFileName().toString().endsWith(".csv"))
                            .findFirst();
                }

                if (match.isEmpty()) {
                    return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                            .body("Erro ao gerar arquivo CSV");
                }

                csvPath = match.get();
            }

            Resource resource = new UrlResource(csvPath.toUri());
            return ResponseEntity.ok()
                    .header("Content-Disposition", "attachment; filename=\"" + csvPath.getFileName().toString() + "\"")
                    .contentType(MediaType.parseMediaType("text/csv"))
                    .body(resource);

        } catch (Exception e) {
            return ResponseEntity.internalServerError()
                    .body("Erro ao processar requisição: " + e.getMessage());
        }
    }

}
