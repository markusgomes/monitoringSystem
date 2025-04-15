#include <WiFi.h>
#include <PubSubClient.h>

const char* redes[][2] = {
  { "Gnomos_Ext_2.4", "Edu@rd00" },
  { "Gnomos_2.4", "Edu@rd00" },
};

const char* mqtt_server = "192.168.15.24";
const int mqtt_port = 1883;
const char* mqtt_user = "servbd";
const char* mqtt_password = "Un1f3sp1";

WiFiClient espClient;
PubSubClient client(espClient);

//Função para conectar ao wifi
void connectToWiFi() {
  int numRedes = sizeof(redes) / sizeof(redes[0]);  // Calcula o número de redes (4)

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
      return;  // Sai da função se conectar
    }
    Serial.println("\nFalha. Tentando próxima rede...");
  }
  // Se todas falharem, reinicia o ESP32
  Serial.println("Todas as redes falharam. Reiniciando...");
  ESP.restart();
}

// Função para reconectar ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Tenta conectar com client ID, usuário e senha
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao broker!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());  // Código de erro
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial
  delay(1000);           // Pequena pausa para estabilizar a conexão serial

  // Inicia a conexão Wi-Fi
  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {

  //MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Exemplo: Publica uma mensagem a cada 10 segundos
  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime > 10000) {
    lastPublishTime = millis();

    String mensagem = "Olá, MQTT!";
    bool publicado = client.publish("topico_de_exemplo", mensagem.c_str());

    if (publicado) {
      Serial.println("Mensagem publicada: " + mensagem);
    } else {
      Serial.println("Falha ao publicar!");
    }
  }
  
}
