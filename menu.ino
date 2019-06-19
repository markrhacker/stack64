#include <M5Stack.h>

void drawonscreenmenu() {
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(50, 50);
  M5.Lcd.printf("SAVED");
  M5.Lcd.fillRect(50, 50, 50, 50, RED);
}
