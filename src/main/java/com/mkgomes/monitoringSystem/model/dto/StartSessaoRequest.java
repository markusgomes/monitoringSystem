package com.mkgomes.monitoringSystem.model.dto;

public class StartSessaoRequest {
    
    private SessaoRequest sessaoRequest;
    private CicloRequest cicloRequest;

    public SessaoRequest getSessaoRequest() {
        return sessaoRequest;
    }
    public void setSessaoRequest(SessaoRequest sessaoRequest) {
        this.sessaoRequest = sessaoRequest;
    }
    public CicloRequest getCicloRequest() {
        return cicloRequest;
    }
    public void setCicloRequest(CicloRequest cicloRequest) {
        this.cicloRequest = cicloRequest;
    }
}
