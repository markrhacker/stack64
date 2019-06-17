// C64 Emulator, works with pic32MX and pic32MZ (SDZL board) under UECIDE
// Pito's version, July 2014
// http://forum.arduino.cc/index.php?topic=193216.msg1793065#msg1793065
// Caps Lock must be ON!
// Use terminal e.g. Teraterm 40x25 char

#include <M5Stack.h>

uint8_t curkey = 0;

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
void printhex(uint16_t val) {
  Serial.print(val, HEX);
  Serial.println();
  M5.Lcd.print(val);
}

void VTposition(uint8_t row, uint8_t col) {
  Serial.write(27);
  Serial.write('[');
  Serial.print(row + 1);
  Serial.write(';');
  Serial.print(col + 1);
  Serial.write('H');
  M5.Lcd.setCursor((col*6) + 40,  (row*6) + 20);
}

void readbutton(){
 if (M5.BtnA.wasReleased()) {
    M5.Lcd.print('A');
  } else if (M5.BtnB.wasReleased()) {
    M5.Lcd.print('B');
  } else if (M5.BtnC.wasReleased()) {
    M5.Lcd.print('C');
  } else if (M5.BtnB.wasReleasefor(700)) {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(0, 0);
  }
}

void readkeyboard() {
  if (digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) {
      uint8_t key_val = Wire.read();                  // receive a byte as character
      if (key_val != 0) {
        curkey = key_val & 0x7F;
        RAM[198] = 1;
        RAM[631] = curkey;
        if (key_val >= 0x20 && key_val < 0x7F) { // ASCII String
          Serial.print((char)key_val);
          //M5.Lcd.print((char)key_val);
        } else {
          Serial.printf("0x%02X ", key_val);
          //M5.Lcd.printf("0x%02X ", key_val);
        }
      }
    }
  }
}

void setup () {
  // initialize the M5Stack object
  M5.begin();
  Wire.begin();
  //M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1); //2 about the write sieze

  pinMode(KEYBOARD_INT, INPUT_PULLUP);

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
  M5.Lcd.fillScreen(BLUE); //clear screen
  delay(1000);

}

int counter = 1;
int effc = 1;

void loop () {

  uint16_t v_address = 0;
  uint16_t last_v_address = 0;
  int eff = 0;

  uint8_t col_c = 0;

  Serial.print("\x1b[H");
  //M5.Lcd.fillScreen(BLUE); //clear screen


  for (uint8_t row = 0; row < 25; row++) {

    for (uint8_t col = 0; col < 40; col++) {


      if (Serial.available()) {
        curkey = Serial.read() & 0x7F;
        RAM[198] = 1;
        RAM[631] = curkey;
      }
      readkeyboard();

      exec6502(100);

      uint8_t petscii = RAM[v_address + 1024];

      if (VRAM[v_address] != petscii) {

        VRAM[v_address] = petscii;

        if (((v_address - last_v_address) > 1)
            || (col_c >= 40)) {

          VTposition(row, col);
          col_c = col;
        }

        if (petscii < 32) petscii = petscii + 64;

        Serial.write(petscii);
        M5.Lcd.setTextColor(WHITE);
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

/*
  uint8_t curkey = 0;

  //extern "C" {
  uint16_t getpc();
  uint8_t getop();
  void exec6502(int32_t tickcount);
  void reset6502();

  void serout(uint8_t val) {
  Serial.write(val);
  }

  uint8_t getkey() {
  return(curkey);
  }

  void clearkey() {
  curkey = 0;
  }

  void printhex(uint16_t val) {
  Serial.print(val, HEX);
  Serial.println();
  }
  //}

  void setup () {
  Serial.begin (115200);
  Serial.println ();
  Serial.println("Press key to starting 6502 CPU...");
  reset6502();
  while (!Serial.available()) {
    delay(100);
  }
  }

  void loop () {
  exec6502(100); //if timing is enabled, this value is in 6502 clock ticks. otherwise, simply instruction count.
  if (Serial.available()) {
    curkey = Serial.read() & 0x7F;
    //Serial.println("receive key");
  }
  }
*/
