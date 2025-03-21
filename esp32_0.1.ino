#define MIC_PIN 32
#define SAMPLES 400
#define DC_OFFSET 1.65       // Offset DC do MAX9814
#define GAIN_DB 60           // Ganho do módulo (60 dB)
#define MIC_SENSITIVITY -46  // Sensibilidade do microfone (ex: -46 dBV/Pa)
#define VREF_SPL 94          // 94 dB SPL = 1 Pa (referência padrão)

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  float sum_squares = 0;
  
  for (int i = 0; i < SAMPLES; i++) {
    float raw_voltage = analogReadMilliVolts(MIC_PIN) / 1000.0; // Leitura em volts
    float ac_signal = raw_voltage - DC_OFFSET; // Remove DC
    sum_squares += ac_signal * ac_signal;
    delayMicroseconds(100); // Taxa de ~10 kHz
  }

  float rms = sqrt(sum_squares / SAMPLES);

  // Cálculo do dB SPL (considerando ganho e sensibilidade)
  float dB_SPL = 20 * log10(rms) + VREF_SPL - MIC_SENSITIVITY - GAIN_DB;

  Serial.print("SPL: ");
  Serial.print(dB_SPL, 1);  
  Serial.println(" dB");
  delay(100);
}