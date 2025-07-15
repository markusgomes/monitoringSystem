package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.MlxData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.MlxTesteEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;

import com.mkgomes.monitoringSystem.repository.MlxTesteRepository;

@Service
public class MlxTesteService implements IMlxTesteService {

    private final MlxTesteRepository mlxTesteRepository;

    public MlxTesteService(MlxTesteRepository mlxTesteRepository) {
        this.mlxTesteRepository = mlxTesteRepository;
    }

    @Override
    public void saveData(MlxData mlxData, SessaoEntity sessao, 
                        CicloEntity cicloEntity) {
        System.out.println("[BANCO]: SALVO_LEITURA_MLX-TESTE");

        MlxTesteEntity mlxTesteEntity = new MlxTesteEntity();
        mlxTesteEntity.setSessao(sessao);
        mlxTesteEntity.setCiclo(cicloEntity);
        mlxTesteEntity.setMillisRelativo(mlxData.getMillisRelativo());
        mlxTesteEntity.setTempAmb(mlxData.getTemp_a());
        mlxTesteEntity.setTempIR(mlxData.getTemp_ir());

        mlxTesteRepository.save(mlxTesteEntity);
    }
    
}
