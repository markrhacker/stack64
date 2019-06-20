#include "FS.h"
#include "SPIFFS.h"
#include <M5Stack.h>

#define FORMAT_SPIFFS_IF_FAILED true

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);
  M5.Lcd.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    M5.Lcd.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    M5.Lcd.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      M5.Lcd.println("  DIR : ");
      Serial.println(file.name());
      M5.Lcd.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      M5.Lcd.print("  FILE: ");
      Serial.print(file.name());
      M5.Lcd.print(file.name());
      Serial.print("\t SIZE: ");
      M5.Lcd.print("\t SIZE: ");
      Serial.println(file.size());
      M5.Lcd.println(file.size());
    }
    file = root.openNextFile();
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  M5.Lcd.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    M5.Lcd.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
    M5.Lcd.println("- file written");
  } else {
    Serial.println("- frite failed");
    M5.Lcd.println("- frite failed");
  }
}

void pushMemory(fs::FS &fs, const char * path) {
  Serial.printf("Memory file I/O with %s\r\n", path);
  M5.Lcd.printf("Memory file I/O with %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    M5.Lcd.println("- failed to open file for writing");
    return;
  }
  // memory array data: uint8_t RAM[RAM_SIZE];
  Serial.print("- writing" );
  M5.Lcd.print("- writing" );
  uint32_t start = millis();
  //file.write(RAM, RAM_SIZE);
  size_t i = 0;
  for(i=0;i<RAM_SIZE;i++) {
    file.write(RAM[i]);
  }
  Serial.println("");
  M5.Lcd.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", RAM_SIZE, end);
  M5.Lcd.println("");
  file.close();
}

void pullMemory(fs::FS &fs, const char * path) {
  Serial.printf("file I/O with %s\r\n", path);
  M5.Lcd.printf("file I/O with %s\r\n", path);

  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    M5.Lcd.println("- failed to open file for reading");
    return;
  }

  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  size_t i = 0;

  // memory array data: uint8_t RAM[RAM_SIZE];
  start = millis();
  len = file.size();
  size_t flen = len;
  Serial.printf("- %u bytes\r\n", flen);
  M5.Lcd.printf("- %u bytes\r\n", flen);
  static uint8_t buf[RAM_SIZE];

  Serial.print("- reading" );
  M5.Lcd.print("- reading" );
  for (i = 0; i < file.size(); i++) //Read upto complete file size
  {
    buf[i] = ((uint8_t)file.read()); 
  }

  Serial.println("");
  M5.Lcd.println("");
  end = millis() - start;
  Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
  M5.Lcd.printf("- %u bytes read in %u ms\r\n", flen, end);
  file.close();

  Serial.println("Memory push");
  M5.Lcd.println("Memory push");
  for (i = 0; i < RAM_SIZE; i++) //Read upto complete file size
  {
    uint8_t tmp = buf[i];
    RAM[i] = tmp;
  }
  
}

void pushCPU(fs::FS &fs, const char * path) {
  Serial.printf("CPU file I/O with %s\r\n", path);
  M5.Lcd.printf("CPU file I/O with %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    M5.Lcd.println("- failed to open file for writing");
    return;
  }
  // memory array data: uint8_t RAM[RAM_SIZE];
  Serial.print("- writing" );
  M5.Lcd.print("- writing" );
  uint32_t start = millis();
  //file.write(RAM, RAM_SIZE);
  size_t i = 0;
  //for(i=0;i<RAM_SIZE;i++) {
  //  file.write(RAM[i]);
  //}
  Serial.println("");
  M5.Lcd.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", RAM_SIZE, end);
  M5.Lcd.println("");
  file.close();
}

void pullCPU(fs::FS &fs, const char * path) {
  Serial.printf("CPU with %s\r\n", path);
  M5.Lcd.printf("CPU I/O with %s\r\n", path);

  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    M5.Lcd.println("- failed to open file for reading");
    return;
  }

  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  size_t i = 0;

  // memory array data: uint8_t RAM[RAM_SIZE];
  start = millis();
  len = file.size();
  size_t flen = len;
  Serial.printf("- %u bytes\r\n", flen);
  M5.Lcd.printf("- %u bytes\r\n", flen);
 // static uint8_t buf[RAM_SIZE];

  Serial.print("- reading" );
  M5.Lcd.print("- reading" );
  //for (i = 0; i < file.size(); i++) //Read upto complete file size
  //{
  //  buf[i] = ((uint8_t)file.read()); 
 // }

  Serial.println("");
  M5.Lcd.println("");
  end = millis() - start;
  Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
  M5.Lcd.printf("- %u bytes read in %u ms\r\n", flen, end);
  file.close();

  Serial.println("CPU push");
  M5.Lcd.println("CPU push");
  //for (i = 0; i < RAM_SIZE; i++) //Read upto complete file size
  //{
  //  uint8_t tmp = buf[i];
  //  RAM[i] = tmp;
 // }
}

void persistenceinit() {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    M5.Lcd.println("SPIFFS Mount Failed");
    return;
  }
  //writeFile(SPIFFS, "/hello.txt", "Hello ");
  //pushMemory(SPIFFS, "/memory.c64");
  //pullMemory(SPIFFS, "/memory.c64");
  listDir(SPIFFS, "/", 0);

  

}
