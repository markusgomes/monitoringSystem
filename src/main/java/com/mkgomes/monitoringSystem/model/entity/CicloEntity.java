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
@Table(name = "ciclos")
public class CicloEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name="id")
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "sessao_id", nullable = false)
    private SessaoEntity sessao;

    @Column(name = "duracao", nullable = false)
    private Integer duracao;

    @Column(name = "temperatura", nullable = false)
    private Float temperatura;

    @Column(name = "quant_cap", nullable = false)
    private Integer quant_cap;

    @Column(name = "data_hora", nullable = false)
    private LocalDateTime dataHoraCriacao = LocalDateTime.now();
    
    public CicloEntity() {}

    public CicloEntity(SessaoEntity sessao, Integer duracao, Float temperatura, Integer quant_cap,
            LocalDateTime dataHoraCriacao) {
        this.sessao = sessao;
        this.duracao = duracao;
        this.temperatura = temperatura;
        this.quant_cap = quant_cap;
        this.dataHoraCriacao = dataHoraCriacao;
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

    public Integer getDuracao() {
        return duracao;
    }

    public void setDuracao(Integer duracao) {
        this.duracao = duracao;
    }

    public Float getTemperatura() {
        return temperatura;
    }

    public void setTemperatura(Float temperatura) {
        this.temperatura = temperatura;
    }

    public Integer getQuant_cap() {
        return quant_cap;
    }

    public void setQuant_cap(Integer quant_cap) {
        this.quant_cap = quant_cap;
    }

    public LocalDateTime getDataHoraCriacao() {
        return dataHoraCriacao;
    }

    public void setDataHoraCriacao(LocalDateTime dataHoraCriacao) {
        this.dataHoraCriacao = dataHoraCriacao;
    }

    
}
