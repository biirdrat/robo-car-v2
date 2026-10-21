/* Libraries
  - RF24 by TMRh20 1.6.0
  - PCF8575 by Rob Tillaart 0.3.0
*/
#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <PCF8575.h> 

// Constant values
constexpr uint16_t PRINT_BUFFER_SIZE = 2000;

// GPIO Pins
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t PCF8575_SDA_PIN = 32;
constexpr uint8_t PCF8575_SCL_PIN = 33;

// PCF8575 PINS
constexpr uint8_t BUT0_PIN = 0;
constexpr uint8_t BUT1_PIN = 1;
constexpr uint8_t BUT2_PIN = 2;
constexpr uint8_t BUT3_PIN = 3;

// General Variables
char printBuffer[PRINT_BUFFER_SIZE];
TwoWire pcf8575Wire(0);
PCF8575 pcf8575(0x20, &pcf8575Wire);

void setup() 
{
  // Begin USB Serial
  Serial.begin(921600);

  printToSerial("\nMain Program Started\n");

  initializeGPIOPins();
  initializePCF8575();
  digitalWrite(LED_PIN, HIGH);
}

void loop() 
{

}

void initializeGPIOPins()
{
  pinMode(LED_PIN, OUTPUT);
}

void initializePCF8575()
{
  pcf8575Wire.begin(PCF8575_SDA_PIN, PCF8575_SCL_PIN);

  while(!pcf8575.begin())
  {
    printToSerial("Failed to initialize PCF8575. Retrying...\n");
    delay(2000);
  }

  // Release pins that are inputs to allow reading
  pcf8575.write(BUT0_PIN, HIGH);
  pcf8575.write(BUT1_PIN, HIGH);
  pcf8575.write(BUT2_PIN, HIGH);
  pcf8575.write(BUT3_PIN, HIGH);

  Serial.print("BUT0 state: ");
  Serial.println(pcf8575.read(BUT0_PIN));
  printToSerial("Successfully initialized PCF8575\n");
}


void printToSerial(const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
  va_end(args);

  Serial.print(printBuffer);
}