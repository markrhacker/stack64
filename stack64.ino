// C64 Emulator
// http://forum.arduino.cc/index.php?topic=193216.msg1793065#msg1793065

#include <M5Stack.h>

uint8_t curkey = 0;
bool onscreenmenu = false;
bool redraw = false;

uint8_t VRAM[1000];

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5
#define RAM_SIZE 30816 //SOMEWHAT LESS THAN 32kB

uint8_t RAM[RAM_SIZE];

uint16_t getpc();
uint8_t getop();
void exec6502(int32_t tickcount);
void reset6502();
void serout(uint8_t val) {
  Serial.write(val);
  M5.Lcd.print(val);
}
uint8_t getkey() {
  return (curkey);
}
void clearkey() {
  curkey = 0;
}
/*
  void printhex(uint16_t val) {
  Serial.print(val, HEX);
  Serial.println();
  M5.Lcd.print(val);
  }*/

void VTposition(uint8_t row, uint8_t col) {
  //Serial.write(27);
  //Serial.write('[');
  //Serial.print(row + 1);
  //Serial.write(';');
  //Serial.print(col + 1);
  //Serial.write('H');
  M5.Lcd.setCursor((col * 6) + 40,  (row * 9) + 5);
}



void drawscreen() {
  
  M5.Lcd.setTextColor(WHITE);
  uint16_t v_address = 0;
  uint16_t last_v_address = 0;
  uint8_t col_c = 0;
  uint8_t petscii;
  
  //M5.Lcd.fillScreen(BLUE); //clear screen

  //do I need a row redraw?
  //if (VRAM[1] != RAM[1 + 1024]) M5.Lcd.fillScreen(BLUE);
  //if (VRAM[1000-49] != RAM[(1000-40) + 1024]) redraw = true;
  if (VRAM[1] != RAM[1 + 1024]) redraw = true;
  if (VRAM[0] != RAM[0 + 1024]) redraw = true;
  
  if (redraw) {
    M5.Lcd.fillScreen(BLUE);
    redraw = false;
  }

  for (uint8_t row = 0; row < 25; row++) {
    for (uint8_t col = 0; col < 40; col++) {
        petscii = RAM[v_address + 1024];
        //if (VRAM[v_address] != petscii) {
        VRAM[v_address] = petscii;
        //  redraw = true;
        //}
        VTposition(row, col);
        if (petscii < 32) petscii = petscii + 64;
        M5.Lcd.print((char)(petscii));
        v_address++;
    }
  }

}

void readbuttons() {

  if (M5.BtnA.wasReleased()) {
    onscreenmenu = !onscreenmenu;
    if (onscreenmenu) {
      drawonscreenmenu();
    }
  }
  else if (M5.BtnB.wasReleased()) {

  }
  else if (M5.BtnC.wasReleased()) {

  }
  else if (M5.BtnA.wasReleasefor(700)) {
    M5.powerOFF();
  }
}

uint8_t coverttouppercase(uint8_t dirtykeyval) {

  uint8_t cleankeyval;
  if (dirtykeyval >= 0x61 && dirtykeyval < 0x7A) {
    cleankeyval = dirtykeyval - 32;
  }
  else {
    cleankeyval = dirtykeyval;
  }
  return cleankeyval;
}

void readkeyboard() {

  /*if (Serial.available()) {
    curkey = Serial.read() & 0x7F;
    RAM[198] = 1;
    RAM[631] = curkey;
    }*/
  if (digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) {
      uint8_t key_val = Wire.read();                  // receive a byte as character
      if (key_val != 0) {
        curkey = coverttouppercase(key_val);// key_val & 0x7F;
        RAM[198] = 1;
        RAM[631] = curkey;
        if (curkey == 0x0D) redraw = true;
        /*if (key_val >= 0x20 && key_val < 0x7F) { // ASCII String
          Serial.print((char)key_val);
          //M5.Lcd.print((char)key_val);
          } else {
          Serial.printf("0x%02X ", key_val);
          //M5.Lcd.printf("0x%02X ", key_val);
          }*/
      }
    }
  }
}

void setup () {
  // initialize the M5Stack object
  M5.begin();                               // M5dstack
  Wire.begin();                             // GPIO
  M5.Lcd.setTextSize(1);                    // LCD text size 1
  pinMode(KEYBOARD_INT, INPUT_PULLUP);      // Keyboaerd

  //WiFi.mode(WIFI_OFF);
  Serial.begin (115200);
  Serial.setDebugOutput(true);
  //delay(10000);//needed to let user open terminal/monitor

  Serial.println ("Prep VRAM..");
  M5.Lcd.println("Prep VRAM..");

  for (int i = 0; i < 1000; i++) {
    VRAM[i] = RAM[i + 1024];
  }

  Serial.println ("Init CPU..");
  M5.Lcd.println ("Init CPU..");
  reset6502();

  Serial.println ("Starting CPU..");
  M5.Lcd.println("Starting CPU..");
  //delay(1000);
  RAM[198] = 0;
  exec6502(200000); 
  M5.Lcd.fillScreen(BLUE); //clear screen
}

int counter = 1;
int effc = 1;

void loop(){

  M5.update();
  readkeyboard();
  exec6502(40*25*200); 
  drawscreen();
  readbuttons();
  
  
}

/*
void loop () {

  uint16_t v_address = 0;
  uint16_t last_v_address = 0;
  int eff = 0;

  uint8_t col_c = 0;

  Serial.print("\x1b[H");
  //M5.Lcd.fillScreen(BLUE); //clear screen

  M5.Lcd.setTextColor(WHITE);

  for (uint8_t row = 0; row < 25; row++) {

    for (uint8_t col = 0; col < 40; col++) {

      readkeyboard();

      exec6502(200); //faster stack allows more ins per loop

      uint8_t petscii = RAM[v_address + 1024];

      if (VRAM[v_address] != petscii) {

        VRAM[v_address] = petscii;

        if (((v_address - last_v_address) > 1) || (col_c >= 40)) {
          VTposition(row, col);
          col_c = col;
        }

        if (petscii < 32) petscii = petscii + 64;

        //Serial.write(petscii);

        M5.Lcd.print((char)(petscii));

        last_v_address = v_address;

      }

      col_c++;
      v_address++;
    }

  }
  readbutton();
  M5.update();

}
*/
