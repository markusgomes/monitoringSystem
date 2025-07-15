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
const char* mqtt_id = "ESP32TESTE";
const char* mqtt_user = "servbd";
const char* mqtt_password = "Un1f3sp1";
const char* controlTopic = "sensores/control";
const char* statusSensoresTopic = "sensoresStatus/teste";
const char* statusESPTopic = "espStatus/teste";
const char* statusNetTopic = "conexoesStatus/teste";
const char* dataDhtTopic = "sensoresDht/teste";
const char* dataMlxTopic = "sensoresMlx/teste";


WiFiClient espClient;
PubSubClient client(espClient);


//BUFFER
struct Leitura {
  time_t epoch;
  char horario[9];
  unsigned long millisRelativo;
  float v1;
  float v2;
};


void taskLeituraDHT(void* parameter);
void taskLeituraMLX(void* parameter);
void publicarMQTT(bool publicarTudo = false);

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


//FUNÇÃO CONECTAR WIFI
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


//FUNÇÃO RECONECTAR BROKER MQTT
void reconnectToBrokerMqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect(mqtt_id, mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao broker!");
      client.subscribe(controlTopic);
      client.publish(statusNetTopic, "[MQTT-TESTE]: CONECTADO");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());  // Código de erro
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}


// FUNÇÃO  FORMATAR HORÁRIO
void horarioAtual(time_t epoch, char* destino) {
  struct tm timeinfo;
  if (!localtime_r(&epoch, &timeinfo)) {
    strcpy(destino, "--:--:--");
    return;
  }
  strftime(destino, 9, "%H:%M:%S", &timeinfo);
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
      xTaskCreatePinnedToCore(taskLeituraDHT, "LeituraDHT", 8192, NULL, 1, &taskHandleDHT, 0);
    }

    if (mlxEnabled && taskHandleMLX == NULL) {
      xTaskCreatePinnedToCore(taskLeituraMLX, "LeituraMLX", 8192, NULL, 1, &taskHandleMLX, 1);
    }

  } else if (message.indexOf("STOP") >= 0) {
    collecting = false;
    dhtEnabled = false;
    mlxEnabled = false;
    maxEnabled = false;
    publicarTudoFlag = true;

    publicarMQTT(true);


    if (taskHandleDHT != NULL) {
      vTaskDelete(taskHandleDHT);
      taskHandleDHT = NULL;
      Serial.println("[ESP32-TESTE-TASK]: TASK_DHT_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-TESTE-TASK]: TASK_DHT_ENCERRADA");
    }

    if (taskHandleMLX != NULL) {
      vTaskDelete(taskHandleMLX);
      taskHandleMLX = NULL;
      Serial.println("[ESP32-TESTE-TASK]: TASK_MLX_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-TESTE-TASK]: TASK_MLX_ENCERRADA");
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
      horarioAtual(dado.epoch, dado.horario);
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
      dado.epoch = time(nullptr);
      horarioAtual(dado.epoch, dado.horario);
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


void publicarBufferMQTT() {
  const int MAX_LINHAS_POR_PUBLICACAO = publicarTudo ? 50 : 10;
  Leitura dado;
  bool dadosPublicados = false;

  // DHT
  if (uxQueueMessagesWaiting(filaDHT) > 0) {
    char csvDHT[TAMANHO_BUFFER_CSV] = "";
    int contadorDHT = 0;
    size_t tamanhoAtualDHT = 0;

    while (contadorDHT < MAX_LINHAS_POR_PUBLICACAO && xQueueReceive(filaDHT, &dado, 0) == pdTRUE) {
      char linha[60];
      int escrito = snprintf(linha, sizeof(linha), "%s,%lu,%.1f,%.1f\n",
                             dado.horario, dado.millisRelativo, dado.v1, dado.v2);

      if (tamanhoAtualDHT + escrito < sizeof(csvDHT) - 1) {
        strcat(csvDHT + tamanhoAtualDHT, linha);
        tamanhoAtualDHT += escrito;
        contadorDHT++;
      } else {
        xQueueSendToFront(filaDHT, &dado, 0);
        Serial.println("[ERRO] Buffer DHT cheio");
        break;
      }
    }

    if (strlen(csvDHT) > 0) {
      if (!client.connected()) reconnectToBrokerMqtt();
      if (client.publish(dataDhtTopic, csvDHT, false)) {
        Serial.println("[MQTT-TESTE]: DHT22-TESTE_PUBLICADO");
        dadosPublicados = true;
      } else {
        Serial.println("[MQTT-TESTE]: ERROR_PUBLICAR_DHT22-TESTE");
      }
    }
  }

  // MLX
  if (uxQueueMessagesWaiting(filaMLX) > 0) {
    char csvMLX[TAMANHO_BUFFER_CSV] = "";
    int contadorMLX = 0;
    size_t tamanhoAtualMLX = 0;

    while (contadorMLX < MAX_LINHAS_POR_PUBLICACAO && xQueueReceive(filaMLX, &dado, 0) == pdTRUE) {
      char linha[60];
      int escrito = snprintf(linha, sizeof(linha), "%s,%lu,%.1f,%.1f\n",
                             dado.horario, dado.millisRelativo, dado.v1, dado.v2);

      if (tamanhoAtualMLX + escrito < sizeof(csvMLX) - 1) {
        strcat(csvMLX + tamanhoAtualMLX, linha);
        tamanhoAtualMLX += escrito;
        contadorMLX++;
      } else {
        xQueueSendToFront(filaMLX, &dado, 0);
        Serial.println("[ERRO] Buffer MLX cheio");
        break;
      }
    }

    if (strlen(csvMLX) > 0) {
      if (!client.connected()) reconnectToBrokerMqtt();
      if (client.publish(dataMlxTopic, csvMLX, false)) {
        Serial.println("[MQTT-TESTE]: MLX90614-TESTE_PUBLICADO");
        dadosPublicados = true;
      } else {
        Serial.println("[MQTT-TESTE]: ERROR_PUBLICAR_MLX90614-TESTE");
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
  vTaskDelay(1);
}


void taskPublicacaoMQTT(void* parameter) {
  while (1) {
    UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("Stack Publicacao: %u\n", stack);

    publicarMQTT(publicarTudoFlag);

    if (publicarTudoFlag) {
      publicarTudoFlag = false;
    }

    static int logCounter = 0;
    if (logCounter++ > 10) {
      Serial.printf("Stack Publicacao: %u\n", stack);
      Serial.printf("Fila DHT: %d | MLX: %d\n",
                    uxQueueMessagesWaiting(filaDHT),
                    uxQueueMessagesWaiting(filaMLX));
      logCounter = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}


void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin(27, 14);
  mlx.begin();

  analogReadResolution(12);
  analogSetPinAttenuation(MAX_PIN, ADC_11db);

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  client.setBufferSize(600);

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Aguardando sincronização NTP...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nNTP sincronizado!");

  Serial.println("\nSISTEMA INICIANDO...");

  filaDHT = xQueueCreate(50, sizeof(Leitura));
  filaMLX = xQueueCreate(50, sizeof(Leitura));

  xTaskCreatePinnedToCore(
    taskPublicacaoMQTT, "taskPublicacao", 10240, NULL, 0, NULL, 0);
}


void loop() {

  //MQTT
  if (!client.connected()) {
    reconnectToBrokerMqtt();
  }
  client.loop();


  //SINAL DE VIDA
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 10000) {
    time_t horaAtual = time(nullptr);
    char hora[9];
    horarioAtual(horaAtual, hora);
    lastHeartbeat = millis();
    char payload[80];
    snprintf(payload, sizeof(payload), "[MQTT-TESTER]: OK | %s | %lus", hora, millis() / 1000);
    client.publish(statusESPTopic, payload);
  }
}