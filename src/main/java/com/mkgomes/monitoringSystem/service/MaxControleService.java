package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.MaxData;
import com.mkgomes.monitoringSystem.model.entity.MaxControleEntity;
import com.mkgomes.monitoringSystem.repository.MaxControleRepository;

@Service
public class MaxControleService implements IMaxControleService{
    
    private final MaxControleRepository maxControleRepository;

    public MaxControleService(MaxControleRepository maxControleRepository) {
        this.maxControleRepository = maxControleRepository;
    }

    public void saveData(MaxData maxData) {
        MaxControleEntity maxControleEntity = new MaxControleEntity();
        maxControleEntity.getSessao();
        maxControleEntity.getMaximo();
        maxControleEntity.getMinimo();

        maxControleRepository.save(maxControleEntity);
    }
}