package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.MaxData;
import com.mkgomes.monitoringSystem.model.entity.MaxEntity;
import com.mkgomes.monitoringSystem.repository.MaxRepository;

@Service
public class MaxService implements IMaxService {
    
    private final MaxRepository maxRepository;

    public MaxService(MaxRepository maxRepository) {
        this.maxRepository = maxRepository;
    }

    public void saveData(MaxData maxData) {
        MaxEntity maxEntity = new MaxEntity();
        maxEntity.getSessao();
        maxEntity.getMaximo();
        maxEntity.getMinimo();

        maxRepository.save(maxEntity);
    }
}
