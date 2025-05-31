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
const char* controlTopic = "sensores/control";

WiFiClient espClient;
PubSubClient client(espClient);


//CONTROLE COLETA
bool dhtEnabled = false;
bool maxEnabled = false;
bool collecting = false;


//FUNÇÃO CONECTAR WIFI
void connectToWiFi() {
  int numRedes = sizeof(redes) / sizeof(redes[0]);

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
    if (client.connect("ESP32Test", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao broker!");
      client.subscribe(controlTopic);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());  // Código de erro
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.toUpperCase();
  if (message.indexOf("START") >= 0) {
    dhtEnabled = false;
    maxEnabled = false;
    collecting = true;

    if (message.indexOf("DHT") >= 0) {
      dhtEnabled = true;
    }

    if (message.indexOf("MAX") >= 0) {
      maxEnabled = true;
    }
  } else if (message.indexOf("STOP") >= 0) {
    collecting = false;
    dhtEnabled = false;
    maxEnabled = false;
    }
}


//FUNÇÕES DE COMPARAÇÃO (QSORT)
int compareAsc(const void* a, const void* b) {
  return (*(float*)a > *(float*)b) ? 1 : -1;
}

int compareDesc(const void* a, const void* b) {
  return (*(float*)a < *(float*)b) ? 1 : -1;
}

//FUNÇÃO ATUALIZAÇÃO RANKINGS
void updateRankings(float dB) {
  // Atualiza os mais altos (ordem crescente)
  if (dB > maxMaximas[0]) {
    maxMaximas[0] = dB;
    qsort(maxMaximas, MAX_SAMPLES, sizeof(float), compareAsc);
  }

  // Atualiza os mais baixos (ordem decrescente)
  if (dB < maxMinimas[0]) {
    maxMinimas[0] = dB;
    qsort(maxMinimas, MAX_SAMPLES, sizeof(float), compareDesc);
  }
}

//FUNÇÃO PUBLICAÇÃO MAX9814
void publishMaxMin() {
  char payloadMax[100];
  char payloadMin[100];

  // Formata 10 maiores (ordem decrescente)
  snprintf(
    payloadMax, sizeof(payloadMax),
    "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
    maxMaximas[9], maxMaximas[8], maxMaximas[7], maxMaximas[6],
    maxMaximas[5], maxMaximas[4], maxMaximas[3], maxMaximas[2],
    maxMaximas[1], maxMaximas[0]);

  // Formata 10 menores (ordem crescente)
  snprintf(
    payloadMin, sizeof(payloadMin),
    "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
    maxMinimas[9], maxMinimas[8], maxMinimas[7], maxMinimas[6],  // Índices 9 a 0
    maxMinimas[5], maxMinimas[4], maxMinimas[3], maxMinimas[2],
    maxMinimas[1], maxMinimas[0]);

  client.publish("sensores/max9814/max", payloadMax);
  client.publish("sensores/max9814/min", payloadMin);

  Serial.println("\n=== RANKINGS MAX9814 (Últimos 5s) ===");
  Serial.print("Top 10 Mais Altos: ");
  Serial.println(payloadMax);
  Serial.print("Top 10 Mais Baixos: ");
  Serial.println(payloadMin);
  Serial.println("================================");
}


void setup() {
  Serial.begin(115200);
  dht.begin();

  analogReadResolution(12);                    // ADC de 12 bits (0-4095)
  analogSetPinAttenuation(MAX_PIN, ADC_11db);  // Atenuação para 3.3V

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  reconnectToBrokerMqtt();

  Serial.println("\nSISTEMA INICIANDO...");
}


void loop() {

  //MQTT
  if (!client.connected()) {
    reconnectToBrokerMqtt();
  }
  client.loop();


  //DHT22
  if (collecting && dhtEnabled) {
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
  }

  //MAX9814
  if (collecting && maxEnabled) {
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
        float dB = 20 * log10(currentRms / 0.006) + 94.0;  // Conversão para dB
        
        updateRankings(dB);
        delay(10);
      }
      
      publishMaxMin();
    }
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
