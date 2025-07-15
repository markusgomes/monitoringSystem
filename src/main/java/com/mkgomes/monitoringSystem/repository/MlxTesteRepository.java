package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.MlxTesteEntity;

public interface MlxTesteRepository extends JpaRepository <MlxTesteEntity, Long> {
    List<MlxTesteEntity> findBySessaoId(Long sessaoId);   
}
