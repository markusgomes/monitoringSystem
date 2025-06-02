package com.mkgomes.monitoringSystem.model.dto;

import java.time.LocalDateTime;

public class SessaoDTO {
    private Long id;
    private int duracao;
    private boolean sensorDht;
    private boolean sensorMax;
    private UsuarioDTO usuario;
    private LocalDateTime dataHoraCriacao;


    public Long getId() {
        return id;
    }
    public void setId(Long id) {
        this.id = id;
    }
    public int getDuracao() {
        return duracao;
    }
    public void setDuracao(int duracao) {
        this.duracao = duracao;
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
    public UsuarioDTO getUsuario() {
        return usuario;
    }
    public void setUsuario(UsuarioDTO usuario) {
        this.usuario = usuario;
    }
    public LocalDateTime getDataHoraCriacao() {
        return dataHoraCriacao;
    }
    public void setDataHoraCriacao(LocalDateTime dataHoraCriacao) {
        this.dataHoraCriacao = dataHoraCriacao;
    }
    
}
