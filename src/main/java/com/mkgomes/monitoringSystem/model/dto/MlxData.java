package com.mkgomes.monitoringSystem.model.dto;

public class MlxData {

    private Long millisRelativo;
    private float temp_a;
    private float temp_ir;

    
    public MlxData() {}

    public MlxData(Long millisRelativo, float temp_a, float temp_ir) {
        this.millisRelativo = millisRelativo;
        this.temp_a = temp_a;
        this.temp_ir = temp_ir;
    }


    public Long getMillisRelativo() {
        return millisRelativo;
    }

    public void setMillisRelativo(Long millisRelativo) {
        this.millisRelativo = millisRelativo;
    }

    public float getTemp_a() {
        return temp_a;
    }

    public void setTemp_a(float temp_a) {
        this.temp_a = temp_a;
    }

    public float getTemp_ir() {
        return temp_ir;
    }

    public void setTemp_ir(float temp_ir) {
        this.temp_ir = temp_ir;
    }
    
    
}
