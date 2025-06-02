package com.mkgomes.monitoringSystem.service;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.mkgomes.monitoringSystem.model.dto.SessaoDTO;
import com.mkgomes.monitoringSystem.model.dto.UsuarioDTO;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.model.entity.UsuarioEntity;
import com.mkgomes.monitoringSystem.repository.SessaoRepository;

import jakarta.persistence.EntityNotFoundException;

@Service
public class SessaoService {

    @Autowired
    private SessaoRepository sessaoRepository;

    @Transactional(readOnly = true)
    public SessaoDTO buscarSessaoComUsuarioDTO(Long id) {
        SessaoEntity sessao = sessaoRepository.findById(id)
            .orElseThrow(() -> new EntityNotFoundException("Sessão não encontrada"));

        // Força carregamento do usuário
        UsuarioEntity usuario = sessao.getUsuario();
        usuario.getNome(); // acessa para inicializar proxy

        // Converte para DTO
        UsuarioDTO usuarioDTO = new UsuarioDTO();
        usuarioDTO.setId(usuario.getId());
        usuarioDTO.setNome(usuario.getNome());
        usuarioDTO.setEmail(usuario.getEmail());

        SessaoDTO sessaoDTO = new SessaoDTO();
        sessaoDTO.setId(sessao.getId());
        sessaoDTO.setDuracao(sessao.getDuracao());
        sessaoDTO.setSensorDht(sessao.isSensorDht());
        sessaoDTO.setSensorMax(sessao.isSensorMax());
        sessaoDTO.setUsuario(usuarioDTO);
        sessaoDTO.setDataHoraCriacao(sessao.getDataHoraCriacao());

        return sessaoDTO;
    }

}
