package com.mkgomes.monitoringSystem.service;

import com.mkgomes.monitoringSystem.model.dto.MlxData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;

public interface IMlxControleService {
    public void saveData(MlxData mlxData, SessaoEntity sessaoEntity, 
                        CicloEntity cicloEntity);
}
