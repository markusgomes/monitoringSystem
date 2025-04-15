#include <WiFi.h>


const char* redes[][2] = {
  {"Gnomos_Ext_2.4", "Edu@rd00"},
  {"Gnomos_2.4", "Edu@rd00"},
};


WiFiClient espClient;


//Função para conectar ao wifi
void connectToWiFi() {
  int numRedes = sizeof(redes) / sizeof(redes[0]); // Calcula o número de redes (4)
  
  for (int i = 0; i < numRedes; i++) {
    Serial.printf("Conectando a: %s\n", redes[i][0]);
    WiFi.begin(redes[i][0], redes[i][1]);
    
    unsigned long inicio = millis();
    // Aguarda até 10 segundos OU até conectar
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado!");
      return; // Sai da função se conectar
    }
    Serial.println("\nFalha. Tentando próxima rede...");
  }
  // Se todas falharem, reinicia o ESP32
  Serial.println("Todas as redes falharam. Reiniciando...");
  ESP.restart();
}


void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial
  delay(1000);           // Pequena pausa para estabilizar a conexão serial

  // Inicia a conexão Wi-Fi
  WiFi.mode(WIFI_STA);
  connectToWiFi();
}

void loop() {
  
}
