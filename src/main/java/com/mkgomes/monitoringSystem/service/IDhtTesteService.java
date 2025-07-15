package com.mkgomes.monitoringSystem.service;

import com.mkgomes.monitoringSystem.model.dto.DhtData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;

public interface IDhtTesteService {

    public void saveData(DhtData dhtData, SessaoEntity sessao, CicloEntity ciclo);
}
