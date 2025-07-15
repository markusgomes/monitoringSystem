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
@Table(name = "sessoes")
public class SessaoEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "id")
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "usuario_id", nullable = false)
    private UsuarioEntity usuario;

    @Column(name = "amostra", nullable = false, length = 250)
    private String amostra;

    @Column(name = "descricao", nullable = false, length = 250)
    private String descricao;

    @Column(name = "data_hora", nullable = false)
    private LocalDateTime dataHoraCriacao = LocalDateTime.now();

    @Column(name = "sensor_dht", nullable = false)
    private boolean sensorDht = false;

    @Column(name = "sensor_max", nullable = false)
    private boolean sensorMax = false;

    @Column(name = "sensor_mlx", nullable = false)
    private boolean sensorMlx = false;

    public SessaoEntity() {}

    public SessaoEntity(UsuarioEntity usuario,String amostra, String descricao, 
                        boolean sensorMlx, boolean sensorDht, boolean sensorMax) {
        this.usuario = usuario;
        this.amostra = amostra;
        this.descricao = descricao;
        this.sensorMlx = sensorMlx;
        this.sensorDht = sensorDht;
        this.sensorMax = sensorMax;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public UsuarioEntity getUsuario() {
        return usuario;
    }

    public void setUsuario(UsuarioEntity usuario) {
        this.usuario = usuario;
    }

    public String getAmostra() {
        return amostra;
    }

    public void setAmostra(String amostra) {
        this.amostra = amostra;
    }

    public String getDescricao() {
        return descricao;
    }

    public void setDescricao(String descricao) {
        this.descricao = descricao;
    }

    public LocalDateTime getDataHoraCriacao() {
        return dataHoraCriacao;
    }

    public void setDataHoraCriacao(LocalDateTime dataHoraCriacao) {
        this.dataHoraCriacao = dataHoraCriacao;
    }

    public boolean isSensorDht() {
        return sensorDht;
    }

    public void setSensorDht(boolean sensorDht) {
        this.sensorDht = sensorDht;
    }

    public boolean isSensorMax() {
        return sensorMax;
    }

    public void setSensorMax(boolean sensorMax) {
        this.sensorMax = sensorMax;
    }

    public boolean isSensorMlx() {
        return sensorMlx;
    }

    public void setSensorMlx(boolean sensorMlx) {
        this.sensorMlx = sensorMlx;
    }
}
