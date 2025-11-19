#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LCD_ADDR 0x27
#define BL 0x08
#define EN 0x04
#define RS 0x01

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

const TickType_t ledTaskExecutionTime = 500 / portTICK_PERIOD_MS;
const TickType_t counterTaskExecutionTime = 2000 / portTICK_PERIOD_MS;
const TickType_t alphabetTaskExecutionTime = 13000 / portTICK_PERIOD_MS;

volatile TickType_t remainingLedTime = ledTaskExecutionTime;
volatile TickType_t remainingCounterTime = counterTaskExecutionTime;
volatile TickType_t remainingAlphabetTime = alphabetTaskExecutionTime;

TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t counterTaskHandle = NULL;
TaskHandle_t alphabetTaskHandle = NULL;

const int ledPin = 2;
int currentCount = 1;
char currentChar = 'A';

static inline void i2cW(uint8_t b) {
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(b);
  Wire.endTransmission();
}
static inline void wr(uint8_t v, bool d) {
  uint8_t base = BL | (d ? RS : 0);
  uint8_t upper = base | (v & 0xF0);
  i2cW(upper | EN);
  i2cW(upper);
  uint8_t lower = base | ((v << 4) & 0xF0);
  i2cW(lower | EN);
  i2cW(lower);
}

void lcdInit() {
  lcd.init();
  delay(2);
  i2cW(BL); // turn on backlight
  wr(0x01, false); // clear screen
  delay(2);
  wr(0x80, false); //set cursor to top right of screen
}

void lcdPrint(const String& s) {
  wr(0x01, false); // clear screen
  delay(5);
  wr(0x80, false); //set cursor to top right of screen
  for (int i=0; i < s.length() && i < 16; i++) {
    wr((uint8_t)s[i], true);
  }
  if (s.length() > 16) {
    wr(0xC0, false); // move to start of second line
    for (int i=16; i < s.length() && i < 32; i++) {
      wr((uint8_t)s[i], true);
    }
  }
}

// Increased timeSlice to slow tasks down more visibly
const TickType_t timeSlice = 1000 / portTICK_PERIOD_MS;

void ledTask(void* arg) {
  bool state = false;
  pinMode(ledPin, OUTPUT);  
  digitalWrite(ledPin, LOW); //turn off pin

  while(1) {
    state = !state;
    digitalWrite(ledPin, state ? HIGH : LOW); //alternating high and low states

    if (remainingLedTime > 0) { //slowly increment time down by time slice
      remainingLedTime = (remainingLedTime > timeSlice) ? (remainingLedTime - timeSlice) : 0;  // if no time remaining set to 0
    }

    if (remainingLedTime == 0) { //
      remainingLedTime = ledTaskExecutionTime; //reset time
    }

  
    vTaskDelay(timeSlice + 2000);  // Extra delay for slower interleaving
  }
}

void counterTask(void* arg) {
  while(1) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %d", currentCount); //printing count to lcd
    lcdPrint(String(buf));

    currentCount++;
    if (currentCount > 20) 
      currentCount = 1;

    if (remainingCounterTime > 0) { //if not done executing then remaining time is subtracted by time slice.
      remainingCounterTime = (remainingCounterTime > timeSlice) ? (remainingCounterTime - timeSlice) : 0; //set time to 0 
    }

    if (remainingCounterTime == 0) { //reset time after done executing
      remainingCounterTime = counterTaskExecutionTime;
    }

    vTaskDelay(timeSlice + 800);  // Extra delay for slower interleaving
  }
}

void alphabetTask(void* arg) {
  while(1) {
    Serial.print("[Alphabet Task] Char: ");
    Serial.println(currentChar);  //printing character to serial monitor

    currentChar++;
    if (currentChar > 'Z') currentChar = 'A';

    if (remainingAlphabetTime > 0) { //if not done executing then remaining time is subtracted by time slice.
      remainingAlphabetTime = (remainingAlphabetTime > timeSlice) ? (remainingAlphabetTime - timeSlice) : 0; // set time to 0
    }

    if (remainingAlphabetTime == 0) {
      remainingAlphabetTime = alphabetTaskExecutionTime; //reset time
    }

    vTaskDelay(timeSlice + 300);  // Extra delay for slower interleaving
  }
}


void scheduleTasks(void* arg) {
  while(1) {
    TickType_t minRemaining = remainingLedTime;
    if (remainingCounterTime < minRemaining) {
      minRemaining = remainingCounterTime;
    }
    if (remainingAlphabetTime < minRemaining) {
      minRemaining = remainingAlphabetTime;
    }
    vTaskDelay(500);  // Slow polling delay
  }
}


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  while(!Serial);

  Wire.begin(20, 21);

  lcdInit();

  xTaskCreatePinnedToCore(ledTask, "LED Task", 2048, NULL, 1, &ledTaskHandle, 0);
  xTaskCreatePinnedToCore(counterTask, "Counter Task", 4096, NULL, 1, &counterTaskHandle, 0);
  xTaskCreatePinnedToCore(alphabetTask, "Alphabet Task", 2048, NULL, 1, &alphabetTaskHandle, 0);
  xTaskCreatePinnedToCore(scheduleTasks, "Scheduler Task", 4096, NULL, 2, NULL, 0);
}

void loop() {}
