package com.mkgomes.monitoringSystem.model.dto;

import java.util.List;

public class SessaoRequest {

    private Long usuario;
    private String amostra;
    private String descricao;
    private List<String> sensores;
    
    public SessaoRequest(Long usuario, String amostra, String descricao, 
                        List<String> sensores) {
        this.usuario = usuario;
        this.amostra = amostra;
        this.descricao = descricao;
        this.sensores = sensores;
    }

    public Long getUsuario() {
        return usuario;
    }

    public void setUsuario(Long usuario) {
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

    public List<String> getSensores() {
        return sensores;
    }

    public void setSensores(List<String> sensores) {
        this.sensores = sensores;
    }
}