package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.MlxControleEntity;

public interface MlxControleRepository extends JpaRepository <MlxControleEntity, Long> {
    List<MlxControleEntity> findBySessaoId(Long sessaoId);
}
