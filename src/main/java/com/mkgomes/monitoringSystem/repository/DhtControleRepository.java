package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.DhtControleEntity;


public interface DhtControleRepository extends JpaRepository<DhtControleEntity, Long> {
    List<DhtControleEntity> findBySessaoId(Long sessaoId);
}
