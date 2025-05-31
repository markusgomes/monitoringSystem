package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.DhtEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.DhtRepository;

@Service
public class DhtService implements IDhtService {

    private final DhtRepository dhtRepository;

    public DhtService(DhtRepository dhtRepository) {
        this.dhtRepository = dhtRepository;
    }

    @Override
    public void saveData(DhtData dhtData, SessaoEntity sessao) {
        DhtEntity dhtEntity = new DhtEntity();
        dhtEntity.setSessao(sessao);
        dhtEntity.setTemperatura(dhtData.getTemperatura());
        dhtEntity.setUmidade(dhtData.getUmidade());

        dhtRepository.save(dhtEntity);
    }
}
