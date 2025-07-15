package com.mkgomes.monitoringSystem.service;

import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;

public interface ISessaoService {

    SessaoDTO buscarSessaoComUsuarioDTO(Long id);
}
