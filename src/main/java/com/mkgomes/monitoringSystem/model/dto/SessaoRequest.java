package com.mkgomes.monitoringSystem.model.dto;

import java.util.List;

public class SessaoRequest {
    private Long usuario;
    private Integer duracao;
    private List<String> sensores;

    
    public SessaoRequest(Long usuario, Integer duracao, List<String> sensores) {
        this.usuario = usuario;
        this.duracao = duracao;
        this.sensores = sensores;
    }

    public SessaoRequest() {
    }


    public Integer getDuracao() {
        return duracao;
    }

    public void setDuracao(Integer duracao) {
        this.duracao = duracao;
    }

    public List<String> getSensores() {
        return sensores;
    }

    public void setSensores(List<String> sensores) {
        this.sensores = sensores;
    }

    public Long getUsuario() {
        return usuario;
    }

    public void setUsuario(Long usuario) {
        this.usuario = usuario;
    }

}