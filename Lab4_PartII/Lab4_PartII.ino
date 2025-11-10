/**
 * @file Lab4_PartII.ino
 * @author Yehoshua Luna
 * @date 2025-11-10
 * @brief Uses the dual core architecture of the ESP32 to capture and process real time sensor data, with binary semaphores to synchronize tasks. 
 *
 * @version 1.0
 * @details
 * ### Update History
 * - **2025-11-10** (Update 1): Initial framework by Yehoshua.
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
SemaphoreHandle_t lightSemaphore;


// =========================================== Objects ============================================
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);


// ======================================== Task Functions =========================================
void taskLight(void *args) {
  // ====================> TODO: 
  // 1. Initialize Variables 
  // 2. Loop Continuously 
  //  - Read light level from the photoresistor. 
  //  - Take semaphore 
  //  - Calculate the simple moving average and update variables. 
  //  - Give semaphore to signal data is ready. 
}


void taskLCD(void *args) {
  // ====================> TODO: 
  // 1. Initialize Variables 
  // 2. Loop Continuously 
  //  - Wait for semaphore. 
  //  - If data has changed, update the LCD with the new light level and SMA. 
  //  - Give back the semaphore. 
}


void taskAnomaly(void *args) {
  // ====================> TODO: 
  // 1. Loop Continuously
  //  - Wait for semaphore.
  //  - Check if SMA indicates a light anomaly (outside thresholds).
  //  - If an anomaly is detected, flash a LED signal.
  //  - Give back the semaphore. Prime Calculation Task (Core 1)
}


void taskPrime(void *args) {
  // ====================> TODO: 
  // 1. Loop from 2 to 5000 
  //  - Check if the current number is prime. 
  //  - If prime, print the number to the serial monitor.
}


// ============================================ Setup =============================================
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(LED_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT_PULLUP);

  lightSemaphore = xSemaphoreCreateBinary();

  if (lightSemaphore != NULL) {
    //xTaskCreatePinnedToCore
  }
  // ====================> TODO: 
  //         1. Initialize pins, serial, LCD, etc 
  //         2. Create binary semaphore for synchronization of light level data. 
  //         3. Create Tasks 
  //          - Create the `Light Detector Task` and assign it to Core 0. 
  //          - Create `LCD Task` and assign it to Core 0. 
  //          - Create `Anomaly Alarm Task` and assign it to Core 1. 
  //          - Create `Prime Calculation Task` and assign it to Core 1. 
}


// ============================================= Loop ==============================================
void loop() {

}