#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <math.h>
#include <time.h>


//DHT22
const int DHT_PIN = 25;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

//MLX90614
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

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
const char* mqtt_id = "ESP32TESTER";
const char* mqtt_user = "servbd";
const char* mqtt_password = "Un1f3sp1";
const char* controlTopic = "sensores/control";
const char* statusSensoresTopic = "sensoresStatus/tester";
const char* statusESPTopic = "espStatus/tester";
const char* statusNetTopic = "conexoesStatus/tester";
const char* dataDhtTopic = "sensoresDht/tester";
const char* dataMlxTopic = "sensoresMlx/tester";


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
      client.publish(statusNetTopic, "[MQTT-TESTER]: CONECTADO");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());  // Código de erro
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("[mqttCallback] Rodando no core: %d\n", xPortGetCoreID());

  char mensagem[100];
  size_t maxLen = sizeof(mensagem) - 1;
  if (length > maxLen) length = maxLen;

  memcpy(mensagem, payload, length);
  mensagem[length] = '\0';
  strupr(mensagem);

  Serial.printf("[MQTT-COMANDO]: %s\n", mensagem);


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
        Serial.printf("[CONFIG] Intervalo leitura: %lu ms\n", intervaloLeituraMs);
      } else {
        Serial.println("[CONFIG] Valor de intervalo inválido. Ignorado.");
      }
    }

    if (dhtEnabled && taskHandleDHT == NULL) {
      xTaskCreatePinnedToCore(taskLeituraDHT, "LeituraDHT", 10240, NULL, 2, &taskHandleDHT, 1);
    }

    if (mlxEnabled && taskHandleMLX == NULL) {
      xTaskCreatePinnedToCore(taskLeituraMLX, "LeituraMLX", 10240, NULL, 1, &taskHandleMLX, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(50));

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
      Serial.println("[ESP32-TESTER-TASK]: TASK_DHT_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-TESTER-TASK]: TASK_DHT_ENCERRADA");
    }

    if (taskHandleMLX != NULL) {
      vTaskDelete(taskHandleMLX);
      taskHandleMLX = NULL;
      Serial.println("[ESP32-TESTER-TASK]: TASK_MLX_ENCERRADA");
      client.publish(statusESPTopic, "[ESP32-TESTER-TASK]: TASK_MLX_ENCERRADA");
    }
  }
}


void taskLeituraDHT(void* parameter) {
  Serial.printf("[taskLeituraDHT] Rodando no core: %d\n", xPortGetCoreID());
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    if (collecting && dhtEnabled) {
      float temperatura = dht.readTemperature();
      float umidade = dht.readHumidity();

      Leitura dado;
      /*dado.epoch = time(nullptr);*/
      /*horarioAtual(dado.epoch, dado.horario);*/
      dado.millisRelativo = (millis() - millisInicioColeta) / 1000;

      if (!isnan(temperatura) && !isnan(umidade)) {
        dado.v1 = temperatura;
        dado.v2 = umidade;
        /*Serial.printf("[DHT22-TESTER]: LEITURA_OK\n");
        client.publish(statusSensoresTopic, "[DHT22-TESTER]: LEITURA_OK");*/
      } else {
        dado.v1 = 0.0;
        dado.v2 = 0.0;
        /*Serial.println("[DHT22-TESTER]: ERROR_LEITURA");
        client.publish(statusSensoresTopic, "[DHT22-TESTER]: ERROR_LEITURA");
        dht.begin();*/
      }
      xQueueSend(filaDHT, &dado, portMAX_DELAY);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(intervaloLeituraMs));
  }
}


void taskLeituraMLX(void* parameter) {
  Serial.printf("[taskLeituraMLX] Rodando no core: %d\n", xPortGetCoreID());
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    if (collecting && mlxEnabled) {
      float tempA = mlx.readAmbientTempC();
      float tempIR = mlx.readObjectTempC();

      Leitura dado;
      /*dado.epoch = time(nullptr);*
      /*horarioAtual(dado.epoch, dado.horario);*/
      dado.millisRelativo = (millis() - millisInicioColeta) / 1000;

      if (!isnan(tempA) && !isnan(tempIR)) {
        dado.v1 = tempA;
        dado.v2 = tempIR;
        /*Serial.printf("[MLX90614-TESTER]: LEITURA_OK\n");
        client.publish(statusSensoresTopic, "[MLX90614-TESTER]: LEITURA_OK");*/
      } else {
        dado.v1 = 0.0;
        dado.v2 = 0.0;
        /*Serial.println("[MLX90614-TESTER]: ERROR_LEITURA");
        client.publish(statusSensoresTopic, "[MLX90614-TESTER]: ERROR_LEITURA");
        mlx.begin();*/
      }
      xQueueSend(filaMLX, &dado, portMAX_DELAY);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(intervaloLeituraMs));
  }
}


void publicarMQTT(bool publicarTudo) {
  Serial.printf("[publicarMQTT] Rodando no core: %d\n", xPortGetCoreID());
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
        Serial.println("[ERRO] Buffer DHT cheio ou corrompido");
        xQueueSendToFront(filaDHT, &dado, 0);
        break;
      }

      tamanhoAtualDHT += escrito;
      contadorDHT++;
    }

    if (tamanhoAtualDHT > 0) {
      if (!client.connected()) reconnectToBrokerMqtt();
      if (client.publish(dataDhtTopic, csvDHT, false)) {
        Serial.println("[MQTT-TESTER]: DHT22-TESTER_PUBLICADO");
      } else {
        Serial.println("[MQTT-TESTER]: ERROR_PUBLICAR_DHT22-TESTER");
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
      if (!client.connected()) reconnectToBrokerMqtt();
      if (client.publish(dataMlxTopic, csvMLX, false)) {
        Serial.println("[MQTT-TESTER]: MLX90614-TESTER_PUBLICADO");
      } else {
        Serial.println("[MQTT-TESTER]: ERROR_PUBLICAR_MLX90614-TESTER");
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
  vTaskDelay(10);
}


void taskPublicacaoMQTT(void* parameter) {
  Serial.printf("[taskPublicacaoMQTT] Rodando no core: %d\n", xPortGetCoreID());
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
    static int heap_counter = 0;
    if (heap_counter++ > 20) {
      heap_counter = 0;
      Serial.printf("Heap livre: %d bytes\n", esp_get_free_heap_size());
      Serial.printf("Menor heap livre: %d bytes\n", esp_get_minimum_free_heap_size());
      Serial.printf("Heap SPI: %d\n", ESP.getMaxAllocHeap());
      Serial.printf("Min Stack TaskPublicacao: %d\n", uxTaskGetStackHighWaterMark(NULL));
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}


void setup() {
  Serial.begin(115200);
  delay(500);

  esp_reset_reason_t motivo = esp_reset_reason();
  Serial.printf("\nMotivo do último reset: %d - ", motivo);

  switch (motivo) {
    case ESP_RST_POWERON: Serial.println("POWER ON"); break;
    case ESP_RST_EXT: Serial.println("EXTERNAL RESET"); break;
    case ESP_RST_SW: Serial.println("SOFTWARE RESET"); break;
    case ESP_RST_PANIC: Serial.println("PANIC / EXCEPTION"); break;
    case ESP_RST_INT_WDT: Serial.println("INTERNAL WATCHDOG"); break;
    case ESP_RST_TASK_WDT: Serial.println("TASK WATCHDOG"); break;
    case ESP_RST_WDT: Serial.println("WATCHDOG RESET"); break;
    case ESP_RST_DEEPSLEEP: Serial.println("DEEP SLEEP WAKE"); break;
    case ESP_RST_BROWNOUT: Serial.println("BROWNOUT (queda de tensão)"); break;
    case ESP_RST_SDIO: Serial.println("SDIO RESET"); break;
    default: Serial.println("DESCONHECIDO"); break;
  }

  dht.begin();
  Wire.begin(27, 14);
  mlx.begin();

  analogReadResolution(12);                    // ADC de 12 bits (0-4095)
  analogSetPinAttenuation(MAX_PIN, ADC_11db);  // Atenuação para 3.3V

  WiFi.mode(WIFI_STA);
  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  client.setBufferSize(1024);

  /*configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Aguardando sincronização NTP...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nNTP sincronizado!");*/

  Serial.println("\nSISTEMA INICIANDO...");

  filaDHT = xQueueCreate(50, sizeof(Leitura));
  filaMLX = xQueueCreate(50, sizeof(Leitura));

  xTaskCreatePinnedToCore(
    taskPublicacaoMQTT, "taskPublicacao", 10240, NULL, 1, NULL, 1);
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
    /*time_t horaAtual = time(nullptr);*/
    /*char hora[9];*/
    /* horarioAtual(horaAtual, hora);*/
    lastHeartbeat = millis();
    char payload[80];
    snprintf(payload, sizeof(payload), "[MQTT-TESTER]: OK | %lus", millis() / 1000);
    client.publish(statusESPTopic, payload);
  }
}

// FUNÇÃO  FORMATAR HORÁRIO
/*void horarioAtual(time_t epoch, char* destino) {
  Serial.printf("[horarioAtual] Rodando no core: %d\n", xPortGetCoreID());
  struct tm timeinfo;
  if (!localtime_r(&epoch, &timeinfo)) {
    strcpy(destino, "--:--:--");
    return;
  }
  strftime(destino, 9, "%H:%M:%S", &timeinfo);
}*/