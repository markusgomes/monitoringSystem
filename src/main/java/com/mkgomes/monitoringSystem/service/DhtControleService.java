package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.DhtControleEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.DhtControleRepository;

@Service
public class DhtControleService implements IDhtControleService {

    private final DhtControleRepository dhtControleRepository;

    public DhtControleService(DhtControleRepository dhtControleRepository) {
        this.dhtControleRepository = dhtControleRepository;
    }

    @Override
    public void saveData(DhtData dhtData, SessaoEntity sessao) {
        DhtControleEntity dhtControleEntity = new DhtControleEntity();
        dhtControleEntity.setSessao(sessao);
        dhtControleEntity.setTemperatura(dhtData.getTemperatura());
        dhtControleEntity.setUmidade(dhtData.getUmidade());

        dhtControleRepository.save(dhtControleEntity);
    }

}