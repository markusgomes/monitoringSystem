package com.mkgomes.monitoringSystem.model.entity;

import java.time.LocalDateTime;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.FetchType;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.ManyToOne;
import jakarta.persistence.Table;

@Entity
@Table(name = "mlx90614")
public class MlxTesteEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "id")
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "sessao_id", nullable = false)
    private SessaoEntity sessao;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "ciclo_id", nullable = false)
    private CicloEntity ciclo;

    @Column(name = "millis_relativo", nullable = false)
    private Long millisRelativo;

    @Column(name = "temp_a", nullable = false)
    private Float tempAmb;

    @Column(name = "temp_ir", nullable = false)
    private Float tempIR;

    @Column(name = "data_hora", nullable = false)
    private LocalDateTime dataHora = LocalDateTime.now();


    public MlxTesteEntity() {}

    public MlxTesteEntity(SessaoEntity sessao, CicloEntity ciclo, Long millisRelativo, Float tempAmb, Float tempIR,
            LocalDateTime dataHora) {
        this.sessao = sessao;
        this.ciclo = ciclo;
        this.millisRelativo = millisRelativo;
        this.tempAmb = tempAmb;
        this.tempIR = tempIR;
        this.dataHora = dataHora;
    }

    
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public SessaoEntity getSessao() {
        return sessao;
    }

    public void setSessao(SessaoEntity sessao) {
        this.sessao = sessao;
    }

    public CicloEntity getCiclo() {
        return ciclo;
    }

    public void setCiclo(CicloEntity ciclo) {
        this.ciclo = ciclo;
    }

    public Long getMillisRelativo() {
        return millisRelativo;
    }

    public void setMillisRelativo(Long millisRelativo) {
        this.millisRelativo = millisRelativo;
    }

    public Float getTempAmb() {
        return tempAmb;
    }

    public void setTempAmb(Float tempAmb) {
        this.tempAmb = tempAmb;
    }

    public Float getTempIR() {
        return tempIR;
    }

    public void setTempIR(Float tempIR) {
        this.tempIR = tempIR;
    }

    public LocalDateTime getDataHora() {
        return dataHora;
    }

    public void setDataHora(LocalDateTime dataHora) {
        this.dataHora = dataHora;
    }

    
}
