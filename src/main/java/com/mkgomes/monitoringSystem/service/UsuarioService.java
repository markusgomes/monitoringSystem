package com.mkgomes.monitoringSystem.service;

import java.util.Optional;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.entity.UsuarioEntity;
import com.mkgomes.monitoringSystem.repository.UsuarioRepository;

@Service

public class UsuarioService {

    @Autowired
    UsuarioRepository usuarioRepository;

    public UsuarioEntity findUsuarioById(Long id) {
        Optional<UsuarioEntity> usrOptional = usuarioRepository.findById(id);
        if (usrOptional.isPresent()) {
            return usrOptional.get();
        }
        throw new IllegalArgumentException("Id inválido!");
    }

    public UsuarioEntity saveAndUpdateUsuario(UsuarioEntity usuario) {
        if (usuario == null ||
                usuario.getNome() == null) {
            throw new IllegalArgumentException("Error!");
        }
        return usuarioRepository.save(usuario);
    }

    public void delUsuarioById(Long id) {
        usuarioRepository.deleteById(id);
    }

}
