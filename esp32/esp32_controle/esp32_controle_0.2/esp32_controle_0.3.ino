#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <math.h>
#include <time.h>


// DHT22
const int DHT_PIN = 25;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// MLX90614
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// MAX9814
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
const char* mqtt_id = "ESP32CONTROLE";
const char* mqtt_user = "servbd";
const char* mqtt_password = "Un1f3sp1";
const char* controlTopic = "sensores/control";
const char* statusSensoresTopic = "sensoresStatus/controle";
const char* statusESPTopic = "espStatus/controle";
const char* statusNetTopic = "conexoesStatus/controle";
const char* dataDhtTopic = "sensoresDht/controle";
const char* dataMlxTopic = "sensoresMlx/controle";


WiFiClient espClient;
PubSubClient client(espClient);


//BUFFER
struct Leitura {
  unsigned long millisRelativo;
  float v1;
  float v2;
};


void taskLeituraDHT(void* parameter);
void taskLeituraMLX(void* parameter);
void publicarMQTT(bool publicarTudo = false);

SemaphoreHandle_t mutexMQTT;

TaskHandle_t taskHandleDHT = NULL;
TaskHandle_t taskHandleMLX = NULL;

QueueHandle_t filaDHT;
QueueHandle_t filaMLX;


//CONTROLE COLETA
bool dhtEnabled = false;
bool mlxEnabled = false;
bool maxEnabled = false;
bool collecting = false;
bool publicarTudoFlag = false;

unsigned long intervaloLeituraMs = 30000;
unsigned long millisInicioColeta = 0;

const int TAMANHO_BUFFER_CSV = 600;


void connectToWiFi() {
  const int maxTentativasWiFi = 5;
  const int maxTentativasTotal = 10;
  int tentativaAtual = 0;

  int numRedes = sizeof(redes) / sizeof(redes[0]);

  while (tentativaAtual < maxTentativasTotal) {
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
      tentativaAtual++;

      if (tentativaAtual == maxTentativasWiFi) {
        Serial.println("Reiniciando WiFi...");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(1000);
        WiFi.mode(WIFI_STA);
      }

      if (tentativaAtual >= maxTentativasTotal) {
        Serial.println("Todas as tentativas falharam. Reiniciando...");
        delay(2000);
        ESP.restart();
      }
    }
  }
}


void reconnectToBrokerMqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");

    if (xSemaphoreTake(mutexMQTT, pdMS_TO_TICKS(200)) == pdTRUE) {
      bool conectado = client.connect(mqtt_id, mqtt_user, mqtt_password);
      if (conectado) {
        Serial.println("Conectado ao broker!");
        client.subscribe(controlTopic);
        client.publish(statusNetTopic, "[MQTT-CONTROLE]: CONECTADO");
        xSemaphoreGive(mutexMQTT);
        return;
      } else {
        Serial.print("Falha, rc=");
        Serial.print(client.state());
        Serial.println(" Tentando novamente em 5 segundos...");
        xSemaphoreGive(mutexMQTT);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}


void taskMqtt(void* parameter) {
  for (;;) {
    if (!client.connected()) {
      reconnectToBrokerMqtt();
    }

    if (xSemaphoreTake(mutexMQTT, pdMS_TO_TICKS(50)) == pdTRUE) {
      client.loop();
      xSemaphoreGive(mutexMQTT);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char mensagem[100];
  size_t maxLen = sizeof(mensagem) - 1;
  if (length > maxLen) length = maxLen;

  memcpy(mensagem, payload, length);
  mensagem[length] = '\0';
  strupr(mensagem);

  if (strstr(mensagem, "START") != NULL) {
    dhtEnabled = false;
    mlxEnabled = false;
    maxEnabled = false;
    collecting = true;
    millisInicioColeta = millis();

    if (strstr(mensagem, "DHT") != NULL) {
      dhtEnabled = true;
    }

    if (strstr(mensagem, "MLX") != NULL) {
      mlxEnabled = true;
    }

    if (strstr(mensagem, "MAX") != NULL) {
      maxEnabled = true;
    }

    char* ultimaVirgula = strrchr(mensagem, ',');
    if (ultimaVirgula != NULL) {
      int valor = atoi(ultimaVirgula + 1);
      if (valor >= 1 && valor <= 60) {
        intervaloLeituraMs = 60000 / valor;
      }
    }

    if (dhtEnabled && taskHandleDHT == NULL) {
      xTaskCreatePinnedToCore(taskLeituraDHT, "LeituraDHT", 10240, NULL, 2, &taskHandleDHT, 1);
    }

    if (mlxEnabled && taskHandleMLX == NULL) {
      xTaskCreatePinnedToCore(taskLeituraMLX, "LeituraMLX", 10240, NULL, 1, &taskHandleMLX, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(100));

  } else if (strstr(mensagem, "STOP") != NULL) {
    collecting = false;
    dhtEnabled = false;
    mlxEnabled = false;
    maxEnabled = false;
    publicarTudoFlag = true;

    publicarMQTT(true);


    if (taskHandleDHT != NULL) {
      vTaskDelete(taskHandleDHT);
      taskHandleDHT = NULL;
      Serial.println("[ESP32-CONTROLE-TASK]: TASK_DHT_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-CONTROLE-TASK]: TASK_DHT_ENCERRADA");
    }

    if (taskHandleMLX != NULL) {
      vTaskDelete(taskHandleMLX);
      taskHandleMLX = NULL;
      Serial.println("[ESP32-CONTROLE-TASK]: TASK_MLX_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-CONTROLE-TASK]: TASK_MLX_ENCERRADA");
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
      dado.millisRelativo = (millis() - millisInicioColeta) / 1000;

      if (!isnan(temperatura) && !isnan(umidade)) {
        dado.v1 = temperatura;
        dado.v2 = umidade;
      } else {
        dado.v1 = 0.0;
        dado.v2 = 0.0;
      }
      xQueueSend(filaDHT, &dado, portMAX_DELAY);
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

      Leitura dado;
      dado.millisRelativo = (millis() - millisInicioColeta) / 1000;

      if (!isnan(tempA) && !isnan(tempIR)) {
        dado.v1 = tempA;
        dado.v2 = tempIR;
      } else {
        dado.v1 = 0.0;
        dado.v2 = 0.0;
      }
      xQueueSend(filaMLX, &dado, portMAX_DELAY);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(intervaloLeituraMs));
  }
}


void publicarMQTT(bool publicarTudo) {
  const int MAX_LINHAS_POR_PUBLICACAO = publicarTudo ? 50 : 3;
  Leitura dado;

  // DHT
  if (uxQueueMessagesWaiting(filaDHT) > 0) {
    char csvDHT[TAMANHO_BUFFER_CSV] = "";
    size_t tamanhoAtualDHT = 0;
    int contadorDHT = 0;

    while (contadorDHT < MAX_LINHAS_POR_PUBLICACAO && xQueueReceive(filaDHT, &dado, 0) == pdTRUE) {
      int escrito = snprintf(csvDHT + tamanhoAtualDHT, sizeof(csvDHT) - tamanhoAtualDHT,
                             "%lu,%.1f,%.1f\n", dado.millisRelativo, dado.v1, dado.v2);

      if (escrito < 0 || (tamanhoAtualDHT + escrito >= sizeof(csvDHT))) {
        xQueueSendToFront(filaDHT, &dado, 0);
        break;
      }

      tamanhoAtualDHT += escrito;
      contadorDHT++;
    }

    if (tamanhoAtualDHT > 0) {
      if (xSemaphoreTake(mutexMQTT, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!client.connected()) reconnectToBrokerMqtt();
        if (client.publish(dataDhtTopic, csvDHT, false)) {
          Serial.println("[MQTT-CONTROLE]: DHT22-CONTROLE_PUBLICADO");
        } else {
          Serial.println("[MQTT-CONTROLE]: ERROR_PUBLICAR_DHT22-CONTROLE");
        }
        xSemaphoreGive(mutexMQTT);
      }
    }
  }

  // MLX
  if (uxQueueMessagesWaiting(filaMLX) > 0) {
    char csvMLX[TAMANHO_BUFFER_CSV] = "";
    size_t tamanhoAtualMLX = 0;
    int contadorMLX = 0;

    while (contadorMLX < MAX_LINHAS_POR_PUBLICACAO && xQueueReceive(filaMLX, &dado, 0) == pdTRUE) {
      int escrito = snprintf(csvMLX + tamanhoAtualMLX, sizeof(csvMLX) - tamanhoAtualMLX,
                             "%lu,%.1f,%.1f\n", dado.millisRelativo, dado.v1, dado.v2);

      if (escrito < 0 || (tamanhoAtualMLX + escrito >= sizeof(csvMLX))) {
        Serial.println("[ERRO] Buffer MLX cheio ou corrompido");
        xQueueSendToFront(filaMLX, &dado, 0);
        break;
      }

      tamanhoAtualMLX += escrito;
      contadorMLX++;
    }

    if (tamanhoAtualMLX > 0) {
      if (xSemaphoreTake(mutexMQTT, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!client.connected()) reconnectToBrokerMqtt();
        if (client.publish(dataMlxTopic, csvMLX, false)) {
          Serial.println("[MQTT-CONTROLE]: MLX90614-CONTROLE_PUBLICADO");
        } else {
          Serial.println("[MQTT-CONTROLE]: ERROR_PUBLICAR_MLX90614-CONTROLE");
        }
        xSemaphoreGive(mutexMQTT);
      }
    }
  }

  if (publicarTudo) {
    while (xQueueReceive(filaDHT, &dado, 0) == pdTRUE) {
      // Fila DHT esvaziada
    }
    while (xQueueReceive(filaMLX, &dado, 0) == pdTRUE) {
      // Fila MLX esvaziada
    }
  }
}


void taskPublicacaoMQTT(void* parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    bool dado = uxQueueMessagesWaiting(filaDHT) > 0 || uxQueueMessagesWaiting(filaMLX) > 0;

    if (dado || publicarTudoFlag) {
      publicarMQTT(publicarTudoFlag);

      if (publicarTudoFlag) {
        publicarTudoFlag = false;
      }

      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
    } else {
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}


void resetarBarramentoDHT() {
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(10);  // Força nível baixo no barramento
  digitalWrite(DHT_PIN, HIGH);
  delay(10);                       // Sobe o barramento
  pinMode(DHT_PIN, INPUT_PULLUP);  // Volta para o estado normal antes do dht.begin()
}


void resetarBarramentoI2C() {
  pinMode(27, INPUT_PULLUP);  // SDA
  pinMode(14, OUTPUT);        // SCL como saída

  for (int i = 0; i < 9; i++) {
    digitalWrite(14, LOW);
    delayMicroseconds(5);
    digitalWrite(14, HIGH);
    delayMicroseconds(5);
  }

  pinMode(14, INPUT_PULLUP);  // Volta ao modo normal antes de Wire.begin()
}


void taskHeartbeat(void* parameter) {
  char payload[100];
  const int intervaloHeartbeat = 10000;

  for (;;) {
    snprintf(payload, sizeof(payload), "[MQTT_CONTROLE]: ESP32-CONTROLE OK — %lu", millis() / 1000);

    if (xSemaphoreTake(mutexMQTT, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (client.connected()) {
        client.publish(statusESPTopic, payload, false);
      }
      xSemaphoreGive(mutexMQTT);
    }
    vTaskDelay(pdMS_TO_TICKS(intervaloHeartbeat));
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  mutexMQTT = xSemaphoreCreateMutex();

  filaDHT = xQueueCreate(100, sizeof(Leitura));
  filaMLX = xQueueCreate(100, sizeof(Leitura));

  resetarBarramentoI2C();
  delay(100);
  Wire.begin(27, 14);
  mlx.begin();

  resetarBarramentoDHT();
  delay(100);  
  dht.begin();

  analogReadResolution(12);
  analogSetPinAttenuation(MAX_PIN, ADC_11db);

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  client.setBufferSize(1000);

  xTaskCreatePinnedToCore(
    taskPublicacaoMQTT, "taskPublicacao", 10240, NULL, 1, NULL, 0);

  xTaskCreatePinnedToCore(taskMqtt, "MQTT", 10240, NULL, 2, NULL, 1);

  xTaskCreatePinnedToCore(taskHeartbeat, "Heartbeat", 4096, NULL, 1, NULL, 0);

  Serial.println("\nSISTEMA INICIANDO...");
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1));
}