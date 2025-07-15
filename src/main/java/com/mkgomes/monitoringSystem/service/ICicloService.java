package com.mkgomes.monitoringSystem.service;

import java.util.List;

import com.mkgomes.monitoringSystem.model.dto.CicloDTO;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;

public interface ICicloService {
    CicloEntity saveData (CicloEntity ciclo);
    List<CicloDTO> findCiclosBySessaoId (Long id);
}
