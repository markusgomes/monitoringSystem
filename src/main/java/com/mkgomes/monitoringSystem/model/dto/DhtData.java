package com.mkgomes.monitoringSystem.model.dto;

public class DhtData {

    private Long millisRelativo;
    private Float temperatura;
    private Float umidade;
    

    public DhtData() {}
    
    public DhtData(Long millisRelativo, 
                    Float temperatura, Float umidade) {
        this.millisRelativo = millisRelativo;
        this.temperatura = temperatura;
        this.umidade = umidade;
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

    public Long getMillisRelativo() {
        return millisRelativo;
    }

    public void setMillisRelativo(Long millisRelativo) {
        this.millisRelativo = millisRelativo;
    }
}
