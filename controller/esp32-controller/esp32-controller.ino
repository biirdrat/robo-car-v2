/* Libraries
  - RF24 by TMRh20 1.6.0
  - PCF8575 by Rob Tillaart 0.3.0
*/
#include <SPI.h>
#include <RF24.h>
#include <PCF8575.h> 

// Constant values
constexpr uint16_t PRINT_BUFFER_SIZE = 2000;

// GPIO Pins
constexpr uint8_t LED_PIN = 2;

// General Variables
char printBuffer[PRINT_BUFFER_SIZE];

void setup() 
{
  // Begin USB Serial
  Serial.begin(921600);

  digitalWrite(LED_PIN, HIGH);
}

void loop() 
{

}

void initializeGPIOPins()
{
  pinMode(LED_PIN, OUTPUT);
}



void printToSerial(const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
  va_end(args);

  Serial.print(printBuffer);
}