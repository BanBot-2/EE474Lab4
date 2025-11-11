/**
 * @file Lab4_PartII.ino
 * @author Yehoshua Luna
 * @date 2025-11-10
 * @brief Uses the dual-core architecture of the ESP32 to capture and process real-time sensor data.
 *        Synchronizes tasks using binary semaphores.
 *
 * @version 1.0
 * @details
 * ### Update History
 * - **2025-11-11** (Update 2): Tweaked task code and stack size.
 * - **2025-11-10** (Update 1): Initial code by Yehoshua.
 */

// =========================================== Includes ============================================
#include <Wire.h>                  ///< Used to initiate the I2C connection
#include <LiquidCrystal_I2C.h>     ///< For working with the LCD
#include "freertos/FreeRTOS.h"     ///< FreeRTOS base definitions
#include "freertos/task.h"         ///< Task creation and management
#include "freertos/semphr.h"       ///< Semaphore and mutex support

// =========================================== Defines =============================================
#define LED_PIN 2                  ///< GPIO pin connected to LED
#define PHOTO_PIN 1                ///< GPIO pin connected to photoresistor

#define SCL_PIN 47                 ///< I2C clock pin
#define SDA_PIN 48                 ///< I2C data pin
#define LCD_ADDRESS 0x27           ///< I2C address for LCD

// ======================================= Global Variables ========================================
SemaphoreHandle_t xLightSemaphore; ///< Binary semaphore for synchronizing light data access

int light = 0;                     ///< Latest light sensor reading
float average = 0;                 ///< Simple moving average of light readings
bool changed = false;             ///< Flag to indicate if light value changed

// =========================================== Objects =============================================
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2); ///< LCD object with 16x2 dimensions

// ======================================== Task Functions =========================================

/**
 * @brief Reads light sensor and calculates simple moving average.
 * @param args Unused task parameter.
 */
void vTaskLight(void *args) {
  int lightHistory[5];
  int index = 0;
  int sum, raw;

  while (true) {
    raw = analogRead(PHOTO_PIN);

    if (xSemaphoreTake(xLightSemaphore, portMAX_DELAY) == pdTRUE) {
      lightHistory[index] = raw;
      index = (index + 1) % 5;

      sum = 0;
      for (int i = 0; i < 5; i++) {
        sum += lightHistory[i];
      }

      light = raw;
      average = sum / 5.0;
      xSemaphoreGive(xLightSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/**
 * @brief Updates LCD with current light and average values.
 *        Only refreshes display when values change.
 * @param args Unused task parameter.
 */
void vTaskLCD(void *args) {
  float prevAverage = 0.0;
  int prevLight = 0;

  while (true) {
    if (xSemaphoreTake(xLightSemaphore, portMAX_DELAY) == pdTRUE) {
      if (light != prevLight || average != prevAverage) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Light: ");
        lcd.print(light);
        lcd.setCursor(0, 1);
        lcd.print("SMA: ");
        lcd.print((int)average);

        prevLight = light;
        prevAverage = average;
      }
      xSemaphoreGive(xLightSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

/**
 * @brief Detects anomalies in light levels and blinks LED if detected.
 *        Anomaly defined as average light > 3800 or < 300.
 * @param args Unused task parameter.
 */
void vTaskAnomaly(void *args) {
  bool anomaly = false;

  while (true) {
    if (xSemaphoreTake(xLightSemaphore, portMAX_DELAY) == pdTRUE) {
      if (average > 3800 || average < 300) {
        anomaly = true;
      }
      xSemaphoreGive(xLightSemaphore);

      if (anomaly) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          vTaskDelay(pdMS_TO_TICKS(250));
          digitalWrite(LED_PIN, LOW);
          vTaskDelay(pdMS_TO_TICKS(2000));
        }
        anomaly = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

/**
 * @brief Checks if a number is prime.
 * @param num Integer to check.
 * @return true if prime, false otherwise.
 */
bool isPrime(int num) {
  for (int i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Finds and prints all prime numbers between 2 and 5000.
 *        Runs once and deletes itself after completion.
 * @param args Unused task parameter.
 */
void vTaskPrime(void *args) {
  for (int i = 2; i <= 5000; i++) {
    if (isPrime(i)) {
      Serial.print("Prime: ");
      Serial.println(i);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(NULL);
}

// ============================================ Setup =============================================

/**
 * @brief Initializes peripherals, semaphore, and tasks.
 *        Pins tasks to both ESP32 cores.
 */
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(LED_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);

  xLightSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xLightSemaphore);

  if (xLightSemaphore != NULL) {
    xTaskCreatePinnedToCore(vTaskLight, "LightTask", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLCD, "LCDTask", 2048, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(vTaskAnomaly, "AnomalyTask", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(vTaskPrime, "PrimeTask", 2048, NULL, 1, NULL, 1);
  }
}

// ============================================= Loop ==============================================

/**
 * @brief Empty loop. All logic handled by FreeRTOS tasks.
 */
void loop() {}