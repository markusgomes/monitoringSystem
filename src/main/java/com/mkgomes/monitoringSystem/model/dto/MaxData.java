package com.mkgomes.monitoringSystem.model.dto;

public class MaxData {

    private Float maximo;
    private Float minimo;
    

    public MaxData() {}

    public MaxData(Float maximo, Float minimo) {
        this.maximo = maximo;
        this.minimo = minimo;
    }


    public Float getMaximo() {
        return maximo;
    }

    public void setMaximo(Float maximo) {
        this.maximo = maximo;
    }

    public Float getMinimo() {
        return minimo;
    }

    public void setMinimo(Float minimo) {
        this.minimo = minimo;
    }

}
