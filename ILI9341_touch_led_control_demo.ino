#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <driver/ledc.h> 

TFT_eSPI tft = TFT_eSPI();

#define LED_PIN 5
#define LED_CHANNEL LEDC_CHANNEL_0

int brightness = 128;
bool ledState = false;

#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 40

#define SLIDER_X 20
#define SLIDER_Y 200
#define SLIDER_WIDTH 200
#define SLIDER_HEIGHT 10

uint16_t calData[5] = { 0 };

void setLED(int value) {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL, value);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL);
}

void drawSlider() {
  tft.drawRect(SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT, TFT_WHITE);
  int handleX = map(brightness, 0, 255, SLIDER_X, SLIDER_X + SLIDER_WIDTH - 10);
  tft.fillRect(handleX, SLIDER_Y - 4, 10, SLIDER_HEIGHT + 8, TFT_BLUE);
}

void clearSliderArea() {
  tft.fillRect(SLIDER_X, SLIDER_Y - 4, SLIDER_WIDTH, SLIDER_HEIGHT + 8, TFT_BLACK);
}

void drawButton() {
  tft.fillRect(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, ledState ? TFT_RED : TFT_GREEN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(ledState ? BUTTON_X + 55 : BUTTON_X + 50, BUTTON_Y + 10);
  tft.print(ledState ? "ON" : "OFF");
}

void drawUI() {
  tft.fillScreen(TFT_BLACK);
  drawButton();
  if (ledState) {
    drawSlider();
  }
}

void toggleLED() {
  ledState = !ledState;
  setLED(ledState ? brightness : 0);
  drawButton();
  clearSliderArea();
  if (ledState) {
    drawSlider();
  }
}

void handleTouch(uint16_t x, uint16_t y) {

  if (x > BUTTON_X && x < BUTTON_X + BUTTON_WIDTH &&
      y > BUTTON_Y && y < BUTTON_Y + BUTTON_HEIGHT) {
    toggleLED();
    delay(200); 
    return;
  }

  if (ledState && y >= SLIDER_Y - 4 && y <= SLIDER_Y + SLIDER_HEIGHT + 4 &&
      x >= SLIDER_X && x <= SLIDER_X + SLIDER_WIDTH) {
    brightness = constrain(map(x, SLIDER_X, SLIDER_X + SLIDER_WIDTH, 0, 255), 0, 255);
    setLED(brightness);
    clearSliderArea();
    drawSlider();
  }
}

void calibrateTouch() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("Touch Calibration...");
  
  tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 15);
  
  Serial.println("Calibration data:");
  for (int i = 0; i < 5; i++) {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println();
}

void setupPWM() {

  ledc_timer_config_t timer_conf = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_8_BIT,
      .timer_num = LEDC_TIMER_0,
      .freq_hz = 5000,
      .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_conf);

  ledc_channel_config_t channel_conf = {
      .gpio_num = LED_PIN,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LED_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0
  };
  ledc_channel_config(&channel_conf);
}

void setup() {
  Serial.begin(115200);
  
  setupPWM();
  
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  
  calibrateTouch();
  tft.setTouch(calData);
  
  drawUI();
}

void loop() {
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    handleTouch(x, y);
  }
  delay(10);
}