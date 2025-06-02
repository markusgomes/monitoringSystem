package com.mkgomes.monitoringSystem.repository;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.DhtEntity;

public interface DhtRepository extends JpaRepository<DhtEntity, Long> {
    List<DhtEntity> findBySessaoId(Long sessaoId);
}
