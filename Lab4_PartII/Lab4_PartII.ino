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

setup() {
  // ====================> TODO: 
  //         1. Initialize pins, serial, LCD, etc 
  //         2. Create binary semaphore for synchronization of light level data. 
  //         3. Create Tasks 
  //          - Create the `Light Detector Task` and assign it to Core 0. 
  //          - Create `LCD Task` and assign it to Core 0. 
  //          - Create `Anomaly Alarm Task` and assign it to Core 1. 
  //          - Create `Prime Calculation Task` and assign it to Core 1. 
}

loop() {

}
 
Light Detector Task (Core 0) {
  // ====================> TODO: 
  // 1. Initialize Variables 
  // 2. Loop Continuously 
  //  - Read light level from the photoresistor. 
  //  - Take semaphore 
  //  - Calculate the simple moving average and update variables. 
  //  - Give semaphore to signal data is ready. 
}

LCD Task (Core 0) {
  // ====================> TODO: 
  // 1. Initialize Variables 
  // 2. Loop Continuously 
  //  - Wait for semaphore. 
  //  - If data has changed, update the LCD with the new light level and SMA. 
  //  - Give back the semaphore. 
  }

Anomaly Alarm Task (Core 1) {
  // ====================> TODO: 
  // 1. Loop Continuously
  //  - Wait for semaphore.
  //  - Check if SMA indicates a light anomaly (outside thresholds).
  //  - If an anomaly is detected, flash a LED signal.
  //  - Give back the semaphore. Prime Calculation Task (Core 1)
}

Prime Calculation Task (Core 1) {
  // ====================> TODO: 
  // 1. Loop from 2 to 5000 
  //  - Check if the current number is prime. 
  //  - If prime, print the number to the serial monitor.
}