package com.mkgomes.monitoringSystem.model.dto;

import java.time.LocalDateTime;

public class SessaoDTO {
    private Long id;
    private UsuarioDTO usuario;
    private String descricao;
    private String amostra;
    private boolean sensorDht;
    private boolean sensorMax;
    private boolean sensorMlx;
    private LocalDateTime dataHoraCriacao;

    public Long getId() {
        return id;
    }
    public void setId(Long id) {
        this.id = id;
    }
    public UsuarioDTO getUsuario() {
        return usuario;
    }
    public void setUsuario(UsuarioDTO usuario) {
        this.usuario = usuario;
    }
    public String getDescricao() {
        return descricao;
    }
    public void setDescricao(String descricao) {
        this.descricao = descricao;
    }
    public String getAmostra() {
        return amostra;
    }
    public void setAmostra(String amostra) {
        this.amostra = amostra;
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
    public LocalDateTime getDataHoraCriacao() {
        return dataHoraCriacao;
    }
    public void setDataHoraCriacao(LocalDateTime dataHoraCriacao) {
        this.dataHoraCriacao = dataHoraCriacao;
    }
}
