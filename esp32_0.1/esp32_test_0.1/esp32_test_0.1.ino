
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <math.h>
#include <time.h>
#include <vector>


//DHT22
const int DHT_PIN = 25;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);


//MLX90614
Adafruit_MLX90614 mlx = Adafruit_MLX90614();


//BUFFER
struct Leitura {
  time_t epoch;
  String horario;
  unsigned long millisRelativo;
  float v1;
  float v2;
};

std::vector<Leitura> bufferDHT;
std::vector<Leitura> bufferMLX;

unsigned long lastPublishTime = 0;
const unsigned long MQTT_BATCH_INTERVAL = 60000;

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
const char* statusTopic = "sensores/status";

TaskHandle_t taskHandleDHT = NULL;
TaskHandle_t taskHandleMLX = NULL;

WiFiClient espClient;
PubSubClient client(espClient);


//CONTROLE COLETA
bool dhtEnabled = false;
bool mlxEnabled = false;
bool maxEnabled = false;
bool collecting = false;
unsigned long intervaloLeituraMs = 30000;
unsigned long millisInicioColeta = 0;


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
    if (client.connect("ESP32TESTE", mqtt_user, mqtt_password)) {
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

String horarioAtual(time_t epoch) {
  struct tm timeinfo;
  if (!localtime_r(&epoch, &timeinfo)) return "--:--:--";
  char buffer[10];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.toUpperCase();

  if (message.indexOf("START") >= 0) {
    dhtEnabled = false;
    mlxEnabled = false;
    maxEnabled = false;
    collecting = true;
    millisInicioColeta = millis();

    if (message.indexOf("DHT") >= 0) {
      dhtEnabled = true;
    }

    if (message.indexOf("MLX") >= 0) {
      mlxEnabled = true;
    }

    if (message.indexOf("MAX") >= 0) {
      maxEnabled = true;
    }

    int idx = message.lastIndexOf(",");
    if (idx != -1) {
      int valor = message.substring(idx + 1).toInt();
      if (valor >= 1 && valor <= 60) {
        intervaloLeituraMs = 60000 / valor;
        Serial.printf("[CONFIG] Intervalo leitura: %lu ms\n", intervaloLeituraMs);
      }
    }

    if (dhtEnabled && taskHandleDHT == NULL) {
      xTaskCreatePinnedToCore(taskLeituraDHT, "LeituraDHT", 4096, NULL, 1, &taskHandleDHT, 1);
    }

    if (mlxEnabled && taskHandleMLX == NULL) {
      xTaskCreatePinnedToCore(taskLeituraMLX, "LeituraMLX", 4096, NULL, 1, &taskHandleMLX, 1);
    }

  } else if (message.indexOf("STOP") >= 0) {
    collecting = false;
    dhtEnabled = false;
    maxEnabled = false;

    if (!bufferDHT.empty() || !bufferMLX.empty()) {
      publicarBufferMQTT();

      if (taskHandleDHT != NULL) {
        vTaskDelete(taskHandleDHT);
        taskHandleDHT = NULL;
        Serial.println("[TASK] LeituraDHT finalizada.");
      }
    }
  }
}


void taskLeituraDHT(void* parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    if (collecting && dhtEnabled) {
      float temperatura = dht.readTemperature();
      float umidade = dht.readHumidity();

      Leitura dado;
      dado.epoch = time(nullptr);
      dado.horario = horarioAtual(dado.epoch);
      dado.millisRelativo = (millis() - millisInicioColeta) / 1000;

      if (!isnan(temperatura) && !isnan(umidade)) {
        dado.v1 = temperatura;
        dado.v2 = umidade;
        Serial.printf("[DHT22-TESTE] Temperatura: %.1f°C Umidade: %.1f%%\n", temperatura, umidade);
      } else {
        dado.v1 = 0.0;
        dado.v2 = 0.0;
        dht.begin();
        Serial.println("[DHT22-TESTE] ERROR_LEITURA");
        client.publish("sensoresTeste/dht/error", "[DHT22-TESTE] ERROR_LEITURA");
      }
      bufferDHT.push_back(dado);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(intervaloLeituraMs));
  }
}


void taskLeituraMLX(void* parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    if (collecting && mlxEnabled) {
      float tempA = mlx.readAmbientTempC();
      float tempIR = mlx.readObjectTempC();

      if (!isnan(tempA) && !isnan(tempIR)) {
        Leitura dado;
        dado.epoch = time(nullptr);
        dado.horario = horarioAtual(dado.epoch);
        dado.millisRelativo = (millis() - millisInicioColeta) / 1000;
        dado.v1 = tempA;
        dado.v2 = tempIR;
        bufferMLX.push_back(dado);
        Serial.printf("[MLX90614-TESTE] Temp-Amb: %.1f°C Temp-IR: %.1f°C\n", tempA, tempIR);
      } else {
        Serial.println("[MLX90614-TESTE] ERROR_LEITURA");
        client.publish("sensoresTeste/mlx/error", "[MLX90614-TESTE] ERROR_LEITURA");
      }
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(intervaloLeituraMs));
  }
}


void publicarBufferMQTT() {
  const size_t LIMITE_BUFFER = 50;
  const int LINHAS_POR_PUBLICACAO = 10;
  const char* topicoDHT_TESTE = "sensoresTeste/dht/dados";
  const char* topicoErro = "sensoresTeste/dht/error";

  if (bufferDHT.size() > LIMITE_BUFFER) {
    Serial.printf("[MQTT-BUFFER] ERROR_BUFFER_LIMITE-MAX");
    client.publish(topicoErro, "[MQTT-BUFFER] ERROR_BUFFER_LIMITE-MAX");
    bufferDHT.clear();
  }

  if (bufferDHT.empty() && bufferMLX.empty()) {
    Serial.println("[MQTT-BUFFER] ERROR_BUFFER");
    client.publish("sensoresTeste/error", "[MQTT-BUFFER] ERROR_BUFFER");
    return;
  }


  // DHT
  if (!bufferDHT.empty()) {
    for (int i = 0; i < bufferDHT.size(); i += LINHAS_POR_PUBLICACAO) {
      String csv = "";

      for (int j = i; j < i + LINHAS_POR_PUBLICACAO && j < bufferDHT.size(); j++) {
        Leitura& dado = bufferDHT[j];
        csv += dado.horario + "," + String(dado.millisRelativo) + "," + String(dado.v1, 1) + "," + String(dado.v2, 1) + "\n";
      }

      if (client.publish(topicoDHT_TESTE, csv.c_str())) {
        Serial.println("[MQTT] DHT22-TESTE_PUBLICADO");
      } else {
        Serial.println("[MQTT] ERROR_PUBLICAR_DHT22-TESTE");
        client.publish(topicoErro, "[MQTT] ERROR_PUBLICAR_DHT22-TESTE");
      }
    }
    bufferDHT.clear();
  }

  // MLX
  if (!bufferMLX.empty()) {
    String csv = "";
    for (auto& dado : bufferMLX) {
      csv += dado.horario + "," + String(dado.millisRelativo) + "," + String(dado.v1, 1) + "," + String(dado.v2, 1) + "\n";
    }

    if (client.publish("sensoresTeste/mlx/dados", csv.c_str())) {
      Serial.println("[MQTT] MLX90614-TESTE_PUBLICADO");
      bufferMLX.clear();
    } else {
      Serial.println("[MQTT] ERROR_PUBLICAR_MLX90614-TESTE");
      client.publish("sensoresTeste/mlx/error", "[MQTT] ERROR_PUBLICAR_MLX90614-TESTE");
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

  client.publish("sensoresTeste/max9814/max", payloadMax);
  client.publish("sensoresTeste/max9814/min", payloadMin);

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
  Wire.begin();
  mlx.begin();

  analogReadResolution(12);                    // ADC de 12 bits (0-4095)
  analogSetPinAttenuation(MAX_PIN, ADC_11db);  // Atenuação para 3.3V

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("[ERROR] DHT22_INDISPONIVEL");
    client.publish("sensoresTeste/dht", "[ERROR] DHT22_INDISPONIVEL");
  } else {
    Serial.println("[OK] DHT22_DISPONIVEL");
    client.publish("sensoresTeste/dht", "[OK] DHT22_DISPONIVEL");
  }

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Aguardando sincronização NTP...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nNTP sincronizado!");

  Serial.println("\nSISTEMA INICIANDO...");
}


void loop() {

  //MQTT
  if (!client.connected()) {
    reconnectToBrokerMqtt();
  }
  client.loop();

  unsigned long agora = millis();
  if (collecting && (!bufferDHT.empty() || !bufferMLX.empty())) {
    if (agora - lastPublishTime >= MQTT_BATCH_INTERVAL) {
      lastPublishTime = agora;
      publicarBufferMQTT();
    }
  }

  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 10000) {
    time_t horaAtual = time(nullptr);
    String hora = horarioAtual(horaAtual);
    lastHeartbeat = millis();
    String payload = "[MQTT_TESTE_OK]: " + hora + " | " + String(millis() / 1000) + "s";
    client.publish(statusTopic, payload.c_str());
  }

  static unsigned long lastLog = 0;
  if (millis() - lastLog > 15000) {
    time_t horaAtual = time(nullptr);
    String hora = horarioAtual(horaAtual);
    lastLog = millis();
    String payload = "[MQTT_LOOP_OK]: " + hora + " | " + String(millis() / 1000) + "s";
    client.publish(statusTopic, payload.c_str());
  }
  /*//DHT22
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
      if (client.publish("sensoresControle/dht22", payload)) {
        Serial.printf("[OK] DHT22: %s\n", payload);
      } else {
        Serial.println("[ERRO] Falha ao publicar no MQTT");
      }
    }
  }*/


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
}
