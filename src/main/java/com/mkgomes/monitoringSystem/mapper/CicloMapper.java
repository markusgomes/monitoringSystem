package com.mkgomes.monitoringSystem.mapper;

import com.mkgomes.monitoringSystem.model.dto.CicloDTO;
import com.mkgomes.monitoringSystem.model.entity.CicloEntity;

public class CicloMapper {

    public static CicloDTO toDTO(CicloEntity entity) {
        CicloDTO dto = new CicloDTO();
        dto.setId(entity.getId());
        dto.setDuracao(entity.getDuracao());
        dto.setTemperatura(entity.getTemperatura());
        dto.setDataHoraCriacao(entity.getDataHoraCriacao());
        dto.setQuant_cap(entity.getQuant_cap());
        return dto;
    }

    public static CicloEntity toEntity(CicloDTO dto) {
        CicloEntity entity = new CicloEntity();
        entity.setId(dto.getId());
        entity.setDuracao(dto.getDuracao());
        entity.setTemperatura(dto.getTemperatura());
        entity.setDataHoraCriacao(dto.getDataHoraCriacao());
        entity.setQuant_cap(dto.getQuant_cap());
        return entity;
    }
}
