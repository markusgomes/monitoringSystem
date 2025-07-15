package com.mkgomes.monitoringSystem.service;

import java.util.List;
import java.util.stream.Collectors;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.mapper.CicloMapper;
import com.mkgomes.monitoringSystem.model.dto.CicloDTO;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;

import com.mkgomes.monitoringSystem.repository.CicloRepository;

@Service
public class CicloService implements ICicloService {

    private final CicloRepository cicloRepository;

    public CicloService(CicloRepository cicloRepository) {
        this.cicloRepository = cicloRepository;
    }

    public CicloEntity saveData(CicloEntity ciclo) {
        return cicloRepository.save(ciclo);
    }

    @Override
    public List<CicloDTO> findCiclosBySessaoId(Long sessaoId) {
        List<CicloEntity> ciclos = cicloRepository.findBySessaoId(sessaoId);
        return ciclos.stream()
                .map(CicloMapper::toDTO)
                .collect(Collectors.toList());
    }
}
