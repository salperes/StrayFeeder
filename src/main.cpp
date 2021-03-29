#include <Arduino.h>

#define resetPin 23
#define trigPin 21
#define echoPin 19
#define core0LED 2
#define core1LED 4
HardwareSerial GSM(2);

// Define variables:
long duration;
int distance;

TaskHandle_t taskCore0;
TaskHandle_t taskCore1;

void updateSerial(void);
//void updateSerial(HardwareSerial &);
void printMenu(void);
void serialFlush(void);
void sendSMS(void);
void gsmInit(void);
void resetSim800(void);
void checkButton(void);
void checkDistance(void);

void codeCore0(void *pvParameters);
void codeCore1(void *pvParameters);

void setup()
{

  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(core0LED, OUTPUT);
  digitalWrite(core0LED, LOW);

  pinMode(core1LED, OUTPUT);
  digitalWrite(core1LED, LOW);

  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);

  Serial.begin(115200);
  Serial.println("Starting");
  gsmInit();

  Serial.printf("CPU ID = %d", xPortGetCoreID());
  Serial.println();
  xTaskCreatePinnedToCore(
      codeCore0,   /* Function to implement the task */
      "taskCore0", /* Name of the task */
      10000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &taskCore0,  /* Task handle. */
      0);          /* Core where the task should run */
}

void loop()
{

  //checkDistance();
  digitalWrite(core1LED, HIGH);
  delay(500);
  digitalWrite(core1LED, LOW);
  delay(500);

  checkButton();
}

void codeCore0(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    digitalWrite(core0LED, HIGH);
    //digitalWrite(core1LED, HIGH);
    checkDistance();
    delay(500);
    digitalWrite(core0LED, LOW);
    //digitalWrite(core1LED, LOW);

    delay(10000);
  }
}

void checkDistance()
{

  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance:
  distance = duration * 0.034 / 2;

  // Print the distance on the Serial Monitor (Ctrl+Shift+M):
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");
}

void updateSerial()
{
  delay(500);
  while (GSM.available())
  {
    Serial.write(GSM.read()); //Forward what Serial received to Software Serial Port
  }
  // while (Serial.available())
  // {
  //   GSM.write(Serial.read()); //Forward what Software Serial received to Serial Port
  // }
}

void checkButton()
{

  // put your main code here, to run repeatedly:
  printMenu();
  while (!Serial.available())
  {
    if (GSM.available())
      updateSerial();
  }
  int charRead = Serial.read() - 48;
  Serial.println(charRead);
  switch (charRead)
  {
  case 1:
    Serial.println("Check AT");
    GSM.println("AT");
    updateSerial();
    break;
  case 2:
    Serial.println("Check Registration");
    GSM.println("AT+CMEE=2");
    updateSerial();
    GSM.println("AT+CREG?");
    updateSerial();
    break;
  case 3:
    Serial.println("Check Signal");
    GSM.println("AT+CSQ");
    updateSerial();
    GSM.println("AT+CSCS?");
    updateSerial();

    break;

  case 4:
    Serial.println("Network Scan");
    GSM.println("AT+CNETSCAN");
    // unsigned long timeOut;
    // timeOut=millis();
    // while(!Serial.available())
    // {

    //   if ((millis()-timeOut)>=60000)
    //   {
    //     Serial.println("timeout");
    //   break;
    //   }
    // }
    // updateSerial();
    break;
  case 5:
    Serial.println("SMS Sending");
    sendSMS();
    delay(2000);
    updateSerial();
    //updateSerial();
    break;
  case 6:
    GSM.println("AT+GSN");
    delay(200);
    updateSerial();
    // Serial.print("Phone Number = ");
    // GSM.println("AT+CUSD=1");
    // updateSerial();
    // GSM.println("AT+CPBS=\"ON\"");
    // updateSerial();
    // GSM.println("AT+CPBR=1");
    // updateSerial();
    // GSM.println("AT*101#");
    // updateSerial();
    break;
  case 7:
    Serial.println("Network Status");
    GSM.println("AT+COPS?");
    updateSerial();
    //updateSerial();AT+CBC
    break;
  case 8:
    Serial.println("Battery Status");
    GSM.println("AT+CBC");
    updateSerial();
    break;
  case 9:
    Serial.println("GSM Reset");
    resetSim800();
    updateSerial();

    delay(5000);
    GSM.println("AT");
    updateSerial();

    break;

  default:
    updateSerial();

    break;
  }
  updateSerial();
}

void printMenu()
{
  Serial.println();
  Serial.println("_______________________________");
  Serial.println("1. Check AT");
  Serial.println("2. Check Registration");
  Serial.println("3. Check Signal");
  Serial.println("4. Network Scan");
  Serial.println("5. Send SMS");
  Serial.println("6. Get Phone Number");
  Serial.println("7. Check Network Status");
  Serial.println("8. Battery Status");
  Serial.println("9. Reset GSM");

  serialFlush();
}

void serialFlush()
{
  while (GSM.available())
  {
    GSM.read();
  }
}

void sendSMS()
{
  Serial.println("Sending SMS..."); //Show this message on serial monitor
  GSM.println("AT+CMGF=1");         //Set the module to SMS mode
  delay(100);
  GSM.println("AT+CMGS=\"+905334696207\""); //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  GSM.println("SIM800l is working"); //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  GSM.print((char)26); // (required according to the datasheet)
  delay(500);
  GSM.println();
  Serial.println("Text Sent.");
  delay(500);
}

void resetSim800()
{
  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);
}

void gsmInit()
{
  GSM.begin(115200);
  serialFlush();
  GSM.println("AT");
  updateSerial();
  GSM.println("ATI");
  updateSerial();
  GSM.println("AT+CPIN?");
  updateSerial();

  GSM.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  GSM.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  GSM.println("AT+GSN"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  GSM.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  //GSM.println("AT+CBAND=\"DCS_MODE\""); //Check whether it has registered in the network
  GSM.println("AT+CPOL?");
  updateSerial();

  // GSM.println("AT+CNETSCAN"); //Check whether it has registered in the network
  // delay(30000);
  // updateSerial();

  serialFlush();
  // put your setup code here, to run once:
}