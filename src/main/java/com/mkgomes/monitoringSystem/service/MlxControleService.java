package com.mkgomes.monitoringSystem.service;

import org.springframework.stereotype.Service;

import com.mkgomes.monitoringSystem.model.dto.MlxData;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;
import com.mkgomes.monitoringSystem.model.entity.MlxControleEntity;
import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;
import com.mkgomes.monitoringSystem.repository.MlxControleRepository;

@Service
public class MlxControleService implements IMlxControleService {

    private final MlxControleRepository mlxControleRepository;

    public MlxControleService(MlxControleRepository mlxControleRepository) {
        this.mlxControleRepository = mlxControleRepository;
    }

    @Override
    public void saveData(MlxData mlxData, SessaoEntity sessao, 
                        CicloEntity cicloEntity) {
        System.out.println("[BANCO]: SALVO_LEITURA_MLX-CONTROLE");

        MlxControleEntity mlxControleEntity = new MlxControleEntity();
        mlxControleEntity.setSessao(sessao);
        mlxControleEntity.setCiclo(cicloEntity);
        mlxControleEntity.setMillisRelativo(mlxData.getMillisRelativo());
        mlxControleEntity.setTempAmb(mlxData.getTemp_a());
        mlxControleEntity.setTempIR(mlxData.getTemp_ir());

        mlxControleRepository.save(mlxControleEntity);
    }
    
}
