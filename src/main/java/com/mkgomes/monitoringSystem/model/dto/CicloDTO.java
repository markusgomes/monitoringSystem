package com.mkgomes.monitoringSystem.model.dto;

import java.time.LocalDateTime;

public class CicloDTO {

    private Long id;
    private SessaoDTO sessaoDTO;
    private Float temperatura;
    private Integer duracao;
    private Integer quant_cap;
    private LocalDateTime dataHoraCriacao;

    public CicloDTO() {}
    
    public CicloDTO(Long id, SessaoDTO sessaoDTO, Float temperatura, Integer duracao, Integer quant_cap,
            LocalDateTime dataHoraCriacao) {
        this.id = id;
        this.sessaoDTO = sessaoDTO;
        this.temperatura = temperatura;
        this.duracao = duracao;
        this.quant_cap = quant_cap;
        this.dataHoraCriacao = dataHoraCriacao;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public SessaoDTO getSessaoDTO() {
        return sessaoDTO;
    }

    public void setSessaoDTO(SessaoDTO sessaoDTO) {
        this.sessaoDTO = sessaoDTO;
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

    public LocalDateTime getDataHoraCriacao() {
        return dataHoraCriacao;
    }

    public void setDataHoraCriacao(LocalDateTime dataHoraCriacao) {
        this.dataHoraCriacao = dataHoraCriacao;
    }
    

}
