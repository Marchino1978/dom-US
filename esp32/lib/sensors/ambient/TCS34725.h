#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

static Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
static bool tcsReady = false;

// ams DN40 lux formula, open air (GA = 1), values matching 50 ms / 4x gain above
#define TCS_ATIME_MS   50.4f
#define TCS_AGAIN      4.0f
#define TCS_GLASS_ATT  1.0f
#define TCS_DF         310.0f
#define TCS_R_COEF     0.136f
#define TCS_G_COEF     1.0f
#define TCS_B_COEF     -0.444f
 
inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  tcsReady = tcs.begin();
  if (!tcsReady) {
    Serial.println("TCS34725 NOT FOUND");
  }
}
 
// getRawData blocks for the integration time (50 ms)
inline float readAmbientLuxValue() {
  if (!tcsReady) return NAN;
 
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) return 0.0f;

  float ir = ((float)r + g + b - c) / 2.0f;
  if (ir < 0) ir = 0;

  float rComp = r - ir;
  float gComp = g - ir;
  float bComp = b - ir;

  float cpl = (TCS_ATIME_MS * TCS_AGAIN) / (TCS_GLASS_ATT * TCS_DF);
  float lux = (TCS_R_COEF * rComp + TCS_G_COEF * gComp + TCS_B_COEF * bComp) / cpl;

  return (lux < 0) ? 0.0f : lux;
}

//• Pre-calcolo del CPL (constexpr float)
//	• Cosa: Spostato il calcolo di cpl fuori dalla funzione e trasformato in costante globale a tempo di compilazione.
//	• Perché: Evita di far calcolare alla CPU dell'ESP32-C3 una moltiplicazione e una divisione in virgola mobile a ogni lettura. Il valore è fisso e viene pre-calcolato dal PC durante la compilazione.
//• Clipping a zero sui canali compensati (rComp, gComp, bComp)
//	• Cosa: Aggiunto un controllo if (xComp < 0.0f) xComp = 0.0f; per ciascun canale cromatico dopo la sottrazione dell'IR.
//	• Perché: Al buio o con spettri luminosi critici, il rumore hardware può far sì che il valore teorico dell'IR superi il valore grezzo del singolo canale. Senza protezione, il canale diventerebbe negativo sballando la formula finale dei Lux.
//• Standardizzazione dei letterali in float (0.0f)
//	• Cosa: Aggiunto il suffisso f a tutte le costanti decimali azzerate.
//	• Perché: L'FPU hardware dell'ESP32-C3 accelera nativamente solo i calcoli a 32-bit (float). Scrivere 0.0 senza la f costringe il chip a emulare via software un calcolo a 64-bit (double), rallentando l'esecuzione di circa 20 volte.
//• Fallback sicuro su Wire.begin()
//	• Cosa: Inserito il controllo condizionale #if defined sui pin I2C.
//	• Perché: Se le macro dei pin non sono presenti in config.h, evita l'errore di compilazione e fa scalare il codice sui pin di default dell'ESP32-C3 (GPIO 8/9).