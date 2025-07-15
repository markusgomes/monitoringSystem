package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.DhtTesteEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.DhtTesteRepository;

@Service
public class DhtTesteService implements IDhtTesteService {

    private final DhtTesteRepository dhtRepository;

    public DhtTesteService(DhtTesteRepository dhtRepository) {
        this.dhtRepository = dhtRepository;
    }

    @Override
    public void saveData(DhtData dhtData, SessaoEntity sessao, CicloEntity ciclo) {
        System.out.println("[BANCO]: SALVO_LEITURA_DHT-TESTE");

        DhtTesteEntity dhtEntity = new DhtTesteEntity();
        dhtEntity.setSessao(sessao);
        dhtEntity.setCiclo(ciclo);
        dhtEntity.setMillisRelativo(dhtData.getMillisRelativo());
        dhtEntity.setTemperatura(dhtData.getTemperatura());
        dhtEntity.setUmidade(dhtData.getUmidade());

        dhtRepository.save(dhtEntity);
    }
    
}
