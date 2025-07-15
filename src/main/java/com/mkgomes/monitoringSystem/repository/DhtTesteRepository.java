package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.DhtTesteEntity;

public interface DhtTesteRepository extends JpaRepository <DhtTesteEntity, Long> {
    List<DhtTesteEntity> findBySessaoId(Long sessaoId);
}
