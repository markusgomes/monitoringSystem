package com.mkgomes.monitoringSystem.repository;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.CicloEntity;

public interface CicloRepository extends JpaRepository <CicloEntity, Long> {
    List<CicloEntity> findBySessaoId(Long sessaoId);
    Optional<CicloEntity> findTopBySessaoIdOrderByDataHoraCriacaoDesc(Long sessaoId);
}
