/**
 * @file Lab4_PartII.ino
 * @author Yehoshua Luna
 * @date 2025-11-10
 * @brief Uses the dual core architecture of the ESP32 to capture and process real time sensor data, with binary semaphores to synchronize tasks. 
 *
 * @version 1.0
 * @details
 * ### Update History
 * - **2025-11-10** (Update 1): Initial code by Yehoshua.
 */


// =========================================== Includes ============================================
#include <Wire.h> // Used to initiate the I2C connection
#include <LiquidCrystal_I2C.h> // For working with the LCD
#include "freertos/FreeRTOS.h" // FreeRTOS base definitions
#include "freertos/task.h" // Task creation and management
#include "freertos/semphr.h" // Semaphore and mutex support


// =========================================== Defines ============================================
#define LED_PIN 1
#define PHOTO_PIN 2

#define SCL_PIN 47
#define SDA_PIN 48
#define LCD_ADDRESS 0x27


// ========================================== Variables ===========================================
SemaphoreHandle_t xLightSemaphore;

int lightLevel = 0;
int lightHistory[5] = {0};

float sma = 0;
int index = 0;

bool dataChanged = false;

// =========================================== Objects ============================================
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);


// ======================================== Task Functions =========================================
void vTaskLight(void *args) {
  for (;;) {
    int raw = analogRead(PHOTO_PIN);

    if (xSemaphoreTake(lightSemaphore, portMAX_DELAY)) {
      lightLevel = raw;
      lightHistory[index] = raw;
      index = (index + 1) % 5;

      int sum = 0;
      for (int i = 0; i < 5; i++) sum += lightHistory[i];
      float newSMA = sum / 5.0;

      if (abs(newSMA - sma) > 5) {
        sma = newSMA;
        dataChanged = true;
      }

      xSemaphoreGive(lightSemaphore);
    }
    //vTaskDelay(pdMS_TO_TICKS(500));
  }
  // ====================> TODO: 
  // 1. Initialize Variables 
  // 2. Loop Continuously 
  //  - Read light level from the photoresistor. 
  //  - Take semaphore 
  //  - Calculate the simple moving average and update variables. 
  //  - Give semaphore to signal data is ready. 
}


void vTaskLCD(void *args) {
  while (true) {
    if (xSemaphoreTake(lightSemaphore, portMAX_DELAY) == pdTRUE) {
      if (dataChanged) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Light: ");
        lcd.print(lightLevel);
        lcd.setCursor(0, 1);
        lcd.print("SMA: ");
        lcd.print((int)sma);
        dataChanged = false;
      }
      xSemaphoreGive(lightSemaphore);
    }
    //vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void vTaskAnomaly(void *args) {
  while (true) {
    if (xSemaphoreTake(lightSemaphore, portMAX_DELAY)) {
      if (sma > 3800 || sma < 300) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          vTaskDelay(pdMS_TO_TICKS(250));
          digitalWrite(LED_PIN, LOW);
          vTaskDelay(pdMS_TO_TICKS(250));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
      xSemaphoreGive(lightSemaphore);
    }
    //vTaskDelay(pdMS_TO_TICKS(1000));
  }
  // ====================> TODO:             
  // 1. Loop Continuously
  //  - Wait for semaphore.
  //  - Check if SMA indicates a light anomaly (outside thresholds).
  //  - If an anomaly is detected, flash a LED signal.
  //  - Give back the semaphore. 
}


void vTaskPrime(void *args) {
  for (int i = 2; i <= 5000; i++) {
    for (int j = 0; j * j <= num; j++) { // Only need to test up to sqrt of num
      if (num % j == 0) { // Modulo operator finds remainder
        Serial.print("Prime: ");
        Serial.println(i);
      }
    }
    //vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(NULL); // End task after completion
}


// ============================================ Setup =============================================
void setup() {
  // Initializes buses
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initializes LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Initializes LED and photoresistor pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);

  // Sets semaphore variable to new binary semaphore
  xLightSemaphore = xSemaphoreCreateBinary();

  // If semaphore was successfully created, pin tasks to both cores
  if (xLightSemaphore != NULL) {
    if (xLightSemaphore != NULL) {
    xTaskCreatePinnedToCore(vTaskLight, "LightTask", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskLCD, "LCDTask", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskAnomaly, "AnomalyTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vTaskPrime, "PrimeTask", 4096, NULL, 1, NULL, 1);
  }
}


// ============================================= Loop ==============================================
void loop();