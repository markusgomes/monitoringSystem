package com.mkgomes.monitoringSystem.repository;

import org.springframework.data.jpa.repository.JpaRepository;

import com.mkgomes.monitoringSystem.model.entity.SessaoEntity;

public interface SessaoRepository extends JpaRepository<SessaoEntity, Long> {}
