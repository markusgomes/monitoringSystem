package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.MaxControleEntity;

public interface MaxControleRepository extends JpaRepository<MaxControleEntity, Long> {
    List<MaxControleEntity> findBySessaoId(Long sessaoId);
}
