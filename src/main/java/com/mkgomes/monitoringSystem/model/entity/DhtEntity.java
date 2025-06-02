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
@Table(name = "dht22")

public class DhtEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "id")
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "sessao_id", nullable = false)
    private SessaoEntity sessao;

    @Column(name = "temperatura", nullable = false)
    private Float temperatura;

    @Column(name = "umidade", nullable = false)
    private Float umidade;

    @Column(name = "data_hora", nullable = false)
    private LocalDateTime dataHora = LocalDateTime.now();


    public DhtEntity() {}
    
    public DhtEntity(SessaoEntity sessao, Float temperatura, Float umidade) {
        this.sessao = sessao;
        this.temperatura = temperatura;
        this.umidade = umidade;
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

    public Float getTemperatura() {
        return temperatura;
    }

    public void setTemperatura(Float temperatura) {
        this.temperatura = temperatura;
    }

    public Float getUmidade() {
        return umidade;
    }

    public void setUmidade(Float umidade) {
        this.umidade = umidade;
    }

    public LocalDateTime getDataHora() {
        return dataHora;
    }

    public void setDataHora(LocalDateTime dataHora) {
        this.dataHora = dataHora;
    }
    
}