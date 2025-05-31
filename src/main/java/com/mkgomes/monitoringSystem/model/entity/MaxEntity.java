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
@Table(name = "max9814")

public class MaxEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "id")
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "sessao_id", nullable = false)
    private SessaoEntity sessao;

    @Column(name = "maximo")
    private Float maximo;
    @Column(name = "minimo")
    private float minimo;

    @Column(name = "data_hora")
    private LocalDateTime dataHora = LocalDateTime.now();


    public MaxEntity() {
    }

    public MaxEntity(SessaoEntity sessao, Float maximo, float minimo) {
        this.sessao = sessao;
        this.maximo = maximo;
        this.minimo = minimo;
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

    public Float getMaximo() {
        return maximo;
    }

    public void setMaximo(Float maximo) {
        this.maximo = maximo;
    }

    public float getMinimo() {
        return minimo;
    }

    public void setMinimo(float minimo) {
        this.minimo = minimo;
    }

    public LocalDateTime getDataHora() {
        return dataHora;
    }

    public void setDataHora(LocalDateTime dataHora) {
        this.dataHora = dataHora;
    }

}
