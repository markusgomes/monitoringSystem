#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <math.h>

//DHT22
const int DHT_PIN = 25;
const unsigned long DHT_INTERVAL = 30000;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

//MAX9814
const int MAX_PIN = 32;
const int MAX_SAMPLES = 10;
float maxMaximas[MAX_SAMPLES];
float maxMinimas[MAX_SAMPLES];
const unsigned long MAX_INTERVAL = 5000;
const int SAMPLES_RMS = 200;
const float DC_OFFSET = 1.65;  // Offset DC do MAX9814


//CONF WIFI
const char* redes[][2] = {
  { "Gnomos_Ext_2.4", "Edu@rd00" },
  { "Gnomos_2.4", "Edu@rd00" },
};

//CONF MQTT
const char* mqtt_server = "192.168.15.24";
const int mqtt_port = 1883;
const char* mqtt_user = "servbd";
const char* mqtt_password = "Un1f3sp1";

WiFiClient espClient;
PubSubClient client(espClient);

//FUNÇÃO CONECTAR WIFI
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
      Serial.println("\nConectado com sucesso!");
      Serial.println(redes[i][0]);
      Serial.print("\nEndereço IP: ");
      Serial.println(WiFi.localIP());
      return;
    }
    Serial.println("\nFalha. Tentando próxima rede...");
  }
  // Se todas falharem, reinicia o ESP32
  Serial.println("Todas as redes falharam. Reiniciando...");
  ESP.restart();
}

//FUNÇÃO RECONECTAR BROKER MQTT
void reconnectToBrokerMqtt() {
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

//FUNÇÕES DE COMPARAÇÃO (QSORT)
int compareAsc(const void* a, const void* b) {
  return (*(float*)a > *(float*)b) ? 1 : -1;
}

int compareDesc(const void* a, const void* b) {
  return (*(float*)a < *(float*)b) ? 1 : -1;
}



void setup() {
  Serial.begin(115200);
  dht.begin();

  analogReadResolution(12);                    // ADC de 12 bits (0-4095)
  analogSetPinAttenuation(MAX_PIN, ADC_11db);  // Atenuação para 3.3V

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);

  Serial.println("\nSISTEMA INICIANDO...");
}


void loop() {

  //MQTT
  if (!client.connected()) {
    reconnectToBrokerMqtt();
  }
  client.loop();


  //DHT22
  static unsigned long lastDHT = 0;
  if (millis() - lastDHT >= DHT_INTERVAL) {
    lastDHT = millis();

    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("[ERRO] Falha na leitura do DHT22. REINICIANDO...");
      dht.begin();
      delay(2000);
      return;
    }

    char payload[15];
    snprintf(payload, sizeof(payload), "%.1f,%.1f", temperatura, umidade);

    if (client.publish("sensores/dht22", payload)) {
      Serial.printf("[OK] DHT22: %s\n", payload);
    } else {
      Serial.println("[ERRO] Falha ao publicar no MQTT");
    }
  }

  //MAX9814
  static unsigned long lastMaxUpdate = 0;
  if (millis() - lastMaxUpdate >= MAX_INTERVAL) {
    lastMaxUpdate = millis();

    for (int i = 0; i < MAX_SAMPLES; i++) {
      maxMaximas[i] = 0.0;
      maxMinimas[i] = 150.0;
    }

    unsigned long startMillis = millis();
    while (millis() - startMillis < MAX_INTERVAL) {

      float sum_squares = 0;

      for (int i = 0; i < SAMPLES_RMS; i++) {
        float raw_voltage = analogReadMilliVolts(MAX_PIN) / 1000.0;  // Leitura em volts
        float ac_signal = raw_voltage - DC_OFFSET;                   // Remove DC
        sum_squares += ac_signal * ac_signal;
        delayMicroseconds(100);
      }

      float currentRms = sqrt(sum_squares / SAMPLES_RMS);
      float dB = 20 * log10(currentRms / 0.0001);  // Conversão para dB

      updateRankings(dB);

      delay(10);
    }

    publishMaxMin();
  }


  // Exemplo: Publica uma mensagem a cada 10 segundos
  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime > 10000) {
    lastPublishTime = millis();

    String mensagem = "Olá, MQTT Teste!";
    bool publicado = client.publish("topico_de_exemplo", mensagem.c_str());

    if (publicado) {
      Serial.println("Mensagem publicada: " + mensagem);
    } else {
      Serial.println("Falha ao publicar!");
    }
  }
}
