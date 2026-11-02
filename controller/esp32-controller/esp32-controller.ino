/* Libraries
  - RF24 by TMRh20 1.6.0
  - PCF8575 by Rob Tillaart 0.3.0
*/
#include <Wire.h>
#include <SPI.h>
#include <PCF8575.h> 
#include <RF24.h>

// Constant values
constexpr uint16_t PRINT_BUFFER_SIZE = 2000;
constexpr uint8_t DATA_PAYLOAD_MAX_SIZE = 32;
constexpr uint8_t BT_DATA_MAX_SIZE = 8;

// GPIO Pins
constexpr uint8_t LED_PIN = 2;

// BLUETOOTH PINS
constexpr uint8_t BT_RX_PIN  = 36;
constexpr uint8_t BT_TX_PIN  = 25;

// NRF24 PINS
constexpr uint8_t VSPI_SCK_PIN  = 18;
constexpr uint8_t VSPI_MISO_PIN = 19;
constexpr uint8_t VSPI_MOSI_PIN = 23;
constexpr uint8_t CE_PIN  = 4;
constexpr uint8_t CSN_PIN = 5;

// PCF8575 PINS
constexpr uint8_t PCF8575_SDA_PIN = 32;
constexpr uint8_t PCF8575_SCL_PIN = 33;
constexpr uint8_t BUT0_PIN = 0;
constexpr uint8_t BUT1_PIN = 1;
constexpr uint8_t BUT2_PIN = 2;
constexpr uint8_t BUT3_PIN = 3;

// General Variables
char printBuffer[PRINT_BUFFER_SIZE];
TwoWire pcf8575Wire(0);
PCF8575 pcf8575(0x20, &pcf8575Wire);
constexpr byte radioAddress[6] = "RADIO";
SPIClass vspi(VSPI);
RF24 radioTransceiver(CE_PIN, CSN_PIN, 1000000);
bool spiOk = false;
bool commsOk = false;
uint8_t radioReceiveBuffer[DATA_PAYLOAD_MAX_SIZE];
uint8_t radioSendBuffer[DATA_PAYLOAD_MAX_SIZE];
char btReceiveBuffer[BT_DATA_MAX_SIZE];
char btSendBuffer[BT_DATA_MAX_SIZE];
uint8_t btReceiveIndex = 0;

void setup() 
{
  // Begin USB Serial
  Serial.begin(921600);

  printToSerial("\nMain Program Started\n");

  initializeGPIOPins();
  initializeBluetooth();
  initializePCF8575();
  initializeRadioSPITransceiver();
  digitalWrite(LED_PIN, HIGH);
  Serial1.println("Hello from ESP32");

}

void loop() 
{
  if(radioReceive())
  {
    Serial.println("YES");
    commsOk = true;
  }
}

void initializeGPIOPins()
{
  pinMode(LED_PIN, OUTPUT);
}

void initializeBluetooth()
{
  // Initialize UART1
  Serial1.begin(9600, SERIAL_8N1, BT_RX_PIN, BT_TX_PIN);

  Serial.println("UART1 Started");
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
  pcf8575.write(15, HIGH);

  Serial.print("BUT0 state: ");
  Serial.println(pcf8575.read(BUT0_PIN));
  printToSerial("Successfully initialized PCF8575\n");
}

void initializeRadioSPITransceiver()
{
  vspi.begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN, CSN_PIN);

  while(!radioTransceiver.begin(&vspi)) 
  {
    printToSerial("Failed to initialize NRF24l01 module. Retrying...\n");
    delay(1000);
  }

  radioTransceiver.enableDynamicPayloads();
  radioTransceiver.setCRCLength(RF24_CRC_16);
  radioTransceiver.setChannel(111);
  radioTransceiver.setPALevel(RF24_PA_MIN);
  radioTransceiver.setDataRate(RF24_250KBPS);
  radioTransceiver.setRetries(1, 3);

  // Open both pipes
  radioTransceiver.openWritingPipe(radioAddress);
  radioTransceiver.openReadingPipe(1, radioAddress);

  radioTransceiver.startListening();

  spiOk = true;

  printToSerial("NRF24l01 module initialized successfully.\n");
}

bool reinitializeRadioSPITransceiver()
{
  if(!radioTransceiver.begin(&vspi)) 
  {
    return false;
  }

  radioTransceiver.enableDynamicPayloads();
  radioTransceiver.setCRCLength(RF24_CRC_16);
  radioTransceiver.setChannel(111);
  radioTransceiver.setPALevel(RF24_PA_MIN);
  radioTransceiver.setDataRate(RF24_250KBPS);
  radioTransceiver.setRetries(1, 3);

  // Open both pipes
  radioTransceiver.openWritingPipe(radioAddress);
  radioTransceiver.openReadingPipe(1, radioAddress);


  radioTransceiver.startListening();

  spiOk = true;

  printToSerial("NRF24l01 module reinitialized successfully.\n");
  
  return true;
}

bool radioReceive()
{
    if(!radioTransceiver.available())
    {
        return false;
    }

    size_t len = radioTransceiver.getDynamicPayloadSize();

    if(len == 0)
    {
      printToSerial("SPI read command failed, communication is lost.\n", (int)len);
      spiOk = false;
      return false;
    }
    else if (len > DATA_PAYLOAD_MAX_SIZE)
    {
      printToSerial("Corrupted/Invalid packet received with length: %i\n", (int)len);
      spiOk = false;
      return false;  
    }

    radioTransceiver.read(radioReceiveBuffer, len);
  
    return true;
}

bool radioSend()
{
    // Switch to TX
    radioTransceiver.stopListening();

    bool success = radioTransceiver.write(radioSendBuffer, DATA_PAYLOAD_MAX_SIZE);

    // Switch Back to RX
    radioTransceiver.startListening();

    if(success)
    {
      return true;
    }
    else
    {
      return false;
    }
}

void printToSerial(const char *fmt, ...) 
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(printBuffer, PRINT_BUFFER_SIZE, fmt, args);
  va_end(args);

  Serial.print(printBuffer);
}