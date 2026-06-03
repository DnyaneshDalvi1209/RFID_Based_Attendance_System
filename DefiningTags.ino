#include <SPI.h>
#include <MFRC522.h>

//-----------------------------------------
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;        
//-----------------------------------------
/* Set the block to write data */
int blockNum = 2;  
/* This is the actual data which is 
going to be written into the card */
byte blockData[16] = {"Kuldeep"};  // Change the name you want to store in RFID Tag
//-----------------------------------------
/* Create array to read data from Block */
byte bufferLen = 18;
byte readBlockData[18];  // Fixed variable declaration
//-----------------------------------------
MFRC522::StatusCode status;
//-----------------------------------------

void setup() 
{
  //-----------------------------------------
  // Initialize serial communications with PC
  Serial.begin(115200);  // Set baud rate to 9600
  //-----------------------------------------
  // Initialize SPI bus
  SPI.begin();
  //-----------------------------------------
  // Initialize MFRC522 Module
  mfrc522.PCD_Init();
  Serial.println("Scan a RFID Tag to write data...");
  //-----------------------------------------
}

void loop()
{
  // Prepare the key for authentication
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n*Card Detected*");

  // Print UID of the Card
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("\n");

  // Print type of card
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Write data to block
  Serial.println("\nWriting to Data Block...");
  WriteDataToBlock(blockNum, blockData);

  // Read data from the same block
  Serial.println("\nReading from Data Block...");
  ReadDataFromBlock(blockNum, readBlockData);

  // Print the data read from block
  Serial.print("\nData in Block ");
  Serial.print(blockNum);
  Serial.print(" --> ");
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.print("\n");
}

// Function to write data to an RFID block
void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  // Authenticate block for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Authentication success");

  // Write data to block
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  Serial.println("Data was written into Block successfully");
}

// Function to read data from an RFID block
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  // Authenticate block for reading
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Authentication success");

  // Read data from block
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  Serial.println("Block was read successfully");
}