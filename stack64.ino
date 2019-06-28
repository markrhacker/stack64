// C64 Emulator
// http://forum.arduino.cc/index.php?topic=193216.msg1793065#msg1793065

#include <M5Stack.h>

uint8_t curkey = 0;
bool onscreenmenu = false;
bool redraw = false;
bool mempull = false;
uint8_t slotid;

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5
#define RAM_SIZE 30816 //SOMEWHAT LESS THAN 32kB

uint8_t RAM[RAM_SIZE];
uint8_t VRAM[1000];

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
  M5.Lcd.setCursor((col * 6) + 40,  (row * 9) + 7);
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
 // if (VRAM[1000-49] != RAM[(1000-49) + 1024]) redraw = true;
 // if (VRAM[0] != RAM[0 + 1024]) redraw = true;
 // if (VRAM[1] != RAM[1 + 1024]) redraw = true;
 // if (VRAM[2] != RAM[2 + 1024]) redraw = true;
//  if (VRAM[3] != RAM[3 + 1024]) redraw = true;
//  if (VRAM[4] != RAM[4 + 1024]) redraw = true;
//  if (VRAM[5] != RAM[5 + 1024]) redraw = true;
  
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
    /*
    onscreenmenu = !onscreenmenu;
    if (onscreenmenu) {
      drawonscreenmenu();
    }*/
  uint8_t sid;
  sid = getSlotNumber(SPIFFS);
  Serial.printf("pre slot: %u\r\n", sid);

  sid = sid +1;
  if (sid > 9) sid = 0;
  setSlotNumber(SPIFFS, sid);
  Serial.printf("post slot: %u\r\n", sid);

  M5.Lcd.setCursor(30,20);
  M5.Lcd.setTextSize(4);  
  M5.Lcd.printf("%u", sid);
  M5.Lcd.setTextSize(1);  
  slotid = sid;
  
  //drawonscreenmenu();
  delay(500);

  redraw = true;
  }
  else if (M5.BtnB.wasReleased()) {
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setCursor(0,0);

    uint8_t sid;
    sid = getSlotNumber(SPIFFS);
    Serial.printf("Slot: %u\r\n", sid);
    M5.Lcd.printf("Slot: %u\r\n", sid);

    char str[100];
    listDir(SPIFFS, "/", 0);

    sprintf(str,"/memory.%u",sid);
    Serial.println(str);
    //pullMemory(SPIFFS, "/"+(char)sid"/memory.c64");
    pullMemory(SPIFFS, str);
    
    sprintf(str,"/cpu.%u",sid);
    Serial.println(str);
    //pullCPU(SPIFFS, "/"+(char)sid"/cpu.c64");
    pullCPU(SPIFFS, str);
    
    delay(1000);
    redraw = true;
  }
  else if (M5.BtnC.wasReleased()) {
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setCursor(0,0);

    uint8_t sid;
    sid = getSlotNumber(SPIFFS);
    Serial.printf("Slot: %u\r\n", sid);
    M5.Lcd.printf("Slot: %u\r\n", sid);
    
    char str[100];
    
    sprintf(str,"/memory.%u",sid);
    Serial.println(str); 
    //pushMemory(SPIFFS, "/memory.c64");
    pushMemory(SPIFFS, str);

    sprintf(str,"/cpu.%u",sid);
    Serial.println(str);
    //pushCPU(SPIFFS, "/cpu.c64");
    pushCPU(SPIFFS, str);
    
    listDir(SPIFFS, "/", 0);
    delay(1000);
    redraw = true;
    mempull = true;
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

  Serial.println("");
  M5.Lcd.println("");
  //WiFi.mode(WIFI_OFF);
  Serial.begin (115200);
  Serial.setDebugOutput(true);
  //delay(10000);//needed to let user open terminal/monitor
  
  Serial.println("");
  M5.Lcd.println("");
  Serial.println("Persistence data..");
  M5.Lcd.println("Persistence data..");
  persistenceinit();

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
  //delay(2000);
  RAM[198] = 0;
  exec6502(200000); 
  
  slotid = getSlotNumber(SPIFFS);
  Serial.printf("pre slot: %u\r\n", slotid);
  
  M5.Lcd.fillScreen(BLUE); //clear screen
}

int counter = 1;
int effc = 1;

void drawslot(){
  M5.Lcd.setCursor(300,220);
  M5.Lcd.setTextSize(2);  
  M5.Lcd.printf("%u", slotid);
  M5.Lcd.setTextSize(1);  
  }

bool needredraw() {
  uint16_t v_address = 0;
  //redraw = false;
 for (uint8_t row = 0; row < 25; row++) {
    for (uint8_t col = 0; col < 40; col++) {
        if (VRAM[v_address] != RAM[v_address + 1024]) {
          redraw = true;
        }
        v_address++;
    }
  }
}

void loop(){

  M5.update();
  readkeyboard();
  exec6502(40*25*100); //(40*25*200); 
  needredraw();
  drawscreen();
  readbuttons();
  drawslot();

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
