package com.mkgomes.monitoringSystem.model.dto;

public class CicloRequest {
    
    private Long sessao;
    private Float temperatura;
    private Integer duracao;
    private Integer quant_cap;


    public CicloRequest() {}

    public CicloRequest(Long sessao, Float temperatura, Integer duracao, Integer quant_cap) {
        this.sessao = sessao;
        this.temperatura = temperatura;
        this.duracao = duracao;
        this.quant_cap = quant_cap;
    }
    

    public Long getSessao() {
        return sessao;
    }
    public void setSessao(Long sessao) {
        this.sessao = sessao;
    }
    public Float getTemperatura() {
        return temperatura;
    }
    public void setTemperatura(Float temperatura) {
        this.temperatura = temperatura;
    }
    public Integer getDuracao() {
        return duracao;
    }
    public void setDuracao(Integer duracao) {
        this.duracao = duracao;
    }
    public Integer getQuant_cap() {
        return quant_cap;
    }
    public void setQuant_cap(Integer quant_cap) {
        this.quant_cap = quant_cap;
    }

}
