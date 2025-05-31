package com.mkgomes.monitoringSystem.model.dto;

public class DhtData {
    
    private Float temperatura;
    private Float umidade;

    
    public DhtData() {}
    
    public DhtData(Float temperatura, Float umidade) {
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
    
}
