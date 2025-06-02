package com.mkgomes.monitoringSystem.service;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;

public interface IDhtControleService {
    
    public void saveData(DhtData dhtData, SessaoEntity sessao);
}
