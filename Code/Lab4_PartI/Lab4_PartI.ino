/**
 * @file Lab4_PartI.ino
 * @author Carter Lee
 * @date 2025-11-19
 * @brief Uses the dual-core architecture of the ESP32 to capture and process real-time sensor data.
 *        Synchronizes tasks by giving the task with the least remaining time priority.
 *
 * @version 2.0
 * @details
 * ### Update History
 * - **2025-11-19** (Update 2): Refined scheduling logic and task timing.
 * - **2025-11-17** (Update 1): Initial implementation by Carter.
 */

// =========================================== Includes ============================================
#include <Wire.h>                  ///< Used to initiate the I2C connection
#include <LiquidCrystal_I2C.h>     ///< For working with the LCD
#include <freertos/FreeRTOS.h>     ///< FreeRTOS base definitions
#include <freertos/task.h>         ///< Task creation and management

// =========================================== Defines =============================================
#define LCD_ADDR 0x27              ///< I2C address of the LCD display
#define BL 0x08                    ///< LCD backlight control bit
#define EN 0x04                    ///< LCD enable bit
#define RS 0x01                    ///< LCD register select bit

// =========================================== Objects =============================================
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2); ///< LCD object with 16 columns and 2 rows

// ======================================= Global Variables ========================================
// --- Task Execution Intervals ---
const TickType_t ledTaskExecutionTime = 500 / portTICK_PERIOD_MS;       ///< Interval for LED task
const TickType_t counterTaskExecutionTime = 2000 / portTICK_PERIOD_MS;  ///< Interval for counter task
const TickType_t alphabetTaskExecutionTime = 13000 / portTICK_PERIOD_MS;///< Interval for alphabet task

// --- Remaining Times ---
volatile TickType_t remainingLedTime = ledTaskExecutionTime;            ///< Remaining time for LED task
volatile TickType_t remainingCounterTime = counterTaskExecutionTime;    ///< Remaining time for counter task
volatile TickType_t remainingAlphabetTime = alphabetTaskExecutionTime;  ///< Remaining time for alphabet task

// --- Task Handles ---
TaskHandle_t ledTaskHandle = NULL;                                      ///< Handle for LED task
TaskHandle_t counterTaskHandle = NULL;                                  ///< Handle for counter task
TaskHandle_t alphabetTaskHandle = NULL;                                 ///< Handle for alphabet task

// --- Hardware Pins ---
const int ledPin = 2;                                                   ///< GPIO pin connected to LED

// --- Task State Variables ---
int currentCount = 1;                                                   ///< Current counter value
char currentChar = 'A';                                                 ///< Current alphabet character

// --- Scheduler ---
const TickType_t timeSlice = 1000 / portTICK_PERIOD_MS;                 ///< Time slice for task execution

// ======================================== Helper Functions ========================================
/**
 * @brief Sends a single byte to the LCD over I2C.
 * @param b Byte to send
 */
static inline void i2cW(uint8_t b) {
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(b);
  Wire.endTransmission();
}

/**
 * @brief Writes a byte to the LCD with control over data/command mode.
 * @param v Byte to write
 * @param d True for data, false for command
 */
static inline void wr(uint8_t v, bool d) {
  uint8_t base = BL | (d ? RS : 0);
  uint8_t upper = base | (v & 0xF0);
  i2cW(upper | EN);
  i2cW(upper);
  uint8_t lower = base | ((v << 4) & 0xF0);
  i2cW(lower | EN);
  i2cW(lower);
}

/**
 * @brief Initializes the LCD display.
 */
void lcdInit() {
  lcd.init();
  delay(2);
  i2cW(BL); // turn on backlight
  wr(0x01, false); // clear screen
  delay(2);
  wr(0x80, false); // set cursor to top left
}

/**
 * @brief Prints a string to the LCD, handling line wrapping.
 * @param s String to display
 */
void lcdPrint(const String& s) {
  wr(0x01, false); // clear screen
  delay(5);
  wr(0x80, false); // set cursor to top left
  for (int i = 0; i < s.length() && i < 16; i++) {
    wr((uint8_t)s[i], true);
  }
  if (s.length() > 16) {
    wr(0xC0, false); // move to second line
    for (int i = 16; i < s.length() && i < 32; i++) {
      wr((uint8_t)s[i], true);
    }
  }
}

// ======================================== Task Functions =========================================
/**
 * @brief Task to blink an LED at a fixed interval.
 * @param arg Unused
 */
void ledTask(void* arg) {
  bool state = false;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  while (1) {
    state = !state;
    digitalWrite(ledPin, state ? HIGH : LOW);

    if (remainingLedTime > 0) {
      remainingLedTime = (remainingLedTime > timeSlice) ? (remainingLedTime - timeSlice) : 0;
    }

    if (remainingLedTime == 0) {
      remainingLedTime = ledTaskExecutionTime;
    }

    vTaskDelay(timeSlice + 2000);
  }
}

/**
 * @brief Task to increment and display a counter on the LCD.
 * @param arg Unused
 */
void counterTask(void* arg) {
  while (1) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %d", currentCount);
    lcdPrint(String(buf));

    currentCount++;
    if (currentCount > 20)
      currentCount = 1;

    if (remainingCounterTime > 0) {
      remainingCounterTime = (remainingCounterTime > timeSlice) ? (remainingCounterTime - timeSlice) : 0;
    }

    if (remainingCounterTime == 0) {
      remainingCounterTime = counterTaskExecutionTime;
    }

    vTaskDelay(timeSlice + 800);
  }
}

/**
 * @brief Task to print alphabet characters to the serial monitor.
 * @param arg Unused
 */
void alphabetTask(void* arg) {
  while (1) {
    Serial.print("[Alphabet Task] Char: ");
    Serial.println(currentChar);

    currentChar++;
    if (currentChar > 'Z') currentChar = 'A';

    if (remainingAlphabetTime > 0) {
      remainingAlphabetTime = (remainingAlphabetTime > timeSlice) ? (remainingAlphabetTime - timeSlice) : 0;
    }

    if (remainingAlphabetTime == 0) {
      remainingAlphabetTime = alphabetTaskExecutionTime;
    }

    vTaskDelay(timeSlice + 300);
  }
}

/**
 * @brief Scheduler task to monitor and compare remaining task times.
 * @param arg Unused
 */
void scheduleTasks(void* arg) {
  while (1) {
    TickType_t minRemaining = remainingLedTime;
    if (remainingCounterTime < minRemaining) {
      minRemaining = remainingCounterTime;
    }
    if (remainingAlphabetTime < minRemaining) {
      minRemaining = remainingAlphabetTime;
    }
    vTaskDelay(500);
  }
}

// ======================================== Arduino Functions ======================================
/**
 * @brief Arduino setup function. Initializes peripherals and creates FreeRTOS tasks.
 */
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  while (!Serial);

  Wire.begin(20, 21);
  lcdInit();

  xTaskCreatePinnedToCore(ledTask, "LED Task", 2048, NULL, 1, &ledTaskHandle, 0);
  xTaskCreatePinnedToCore(counterTask, "Counter Task", 4096, NULL, 1, &counterTaskHandle, 0);
  xTaskCreatePinnedToCore(alphabetTask, "Alphabet Task", 2048, NULL, 1, &alphabetTaskHandle, 0);
  xTaskCreatePinnedToCore(scheduleTasks, "Scheduler Task", 4096, NULL, 2, NULL, 0);
}

/**
 * @brief Arduino loop function. Not used in this FreeRTOS-based program.
 */
void loop() {}