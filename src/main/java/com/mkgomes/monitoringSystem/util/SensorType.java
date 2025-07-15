package com.mkgomes.monitoringSystem.util;

import java.util.List;

public enum SensorType {
    
    DHT(List.of(
        "temp_dht_controle", 
        "temp_dht_test",
        "delta_temp_dht", 
        "umidade_controle", 
        "umidade_test",
        "delta_umidade")),

    MLX(List.of(
        "mlx-a_controle", 
        "mlx-a_test", 
        "delta_mlx-a",
        "mlx_ir_controle", 
        "mlx_ir_test",
        "delta_mlx-ir")),

    MAX(List.of(
        "dB_controle", 
        "dB_test"));

    private final List<String> colunas;

    SensorType(List<String> colunas) {
        this.colunas = colunas;
    }

    public List<String> getColunas() {
        return colunas;
    }
}
