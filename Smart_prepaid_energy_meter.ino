#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define buzzer 8
#define relay  A1
#define relay2  A2
#define pulse_in A0

char inchar;
String inputBuffer = "";

int unt_a = 0, unt_b = 0, unt_c = 0, unt_d = 0;
long total_unt = 7;

int price = 0;
long price1 = 0;

int Set = 10;

int pulse = 0;

String phone_no1 = "+918506906704";
String phone_no2 = "+917053540909";

int flag1 = 0, flag2 = 0, flag3 = 0;

void setup()
{
  Serial.begin(9600);
  total_unt = 0;
  price1 = 0;

  // Reset EEPROM values to zero
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  EEPROM.write(3, 0);
  EEPROM.write(4, 0);
  EEPROM.write(10, 0);

  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pulse_in, INPUT);
  attachInterrupt(0, ai0, RISING);

  lcd.begin(16, 2); // Initialize the LCD with 16 columns and 2 rows
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("WELCOME");
  lcd.setCursor(2, 1);
  lcd.print("Energy Meter");
  digitalWrite(buzzer, HIGH);

  Serial.println("Initializing....");
  initModule("AT", "OK", 1000);
  initModule("ATE1", "OK", 1000);
  initModule("AT+CPIN?", "READY", 2000);
  initModule("AT+CMGF=1", "OK", 2000);
  initModule("AT+CNMI=2,2,0,0,0", "OK", 2000);
  Serial.println("Initialized Successfully");
  delay(100);
  sendSMS(phone_no1, "Welcome To Energy Meter");
  sendSMS(phone_no2, "Welcome To Energy Meter");
  digitalWrite(buzzer, LOW);
  lcd.clear();

  if (EEPROM.read(50) == 0) {}
  else {
    Write();
  }

  EEPROM.write(50, 0);

  pulse = EEPROM.read(10);

  Read();
  if (total_unt > 0)
  {
    digitalWrite(relay, HIGH);
    digitalWrite(relay2, HIGH);
  }
}

void loop()
{
  if (Serial.available() > 0)
  {
    
    char receivedChar = Serial.read();

    if (receivedChar == '\r') 
    {
      // 'Enter' key detected, process the received string
      processInput(inputBuffer);
      // Clear the input buffer for the next command
      inputBuffer = "";
    } 
    else 
    {
      // Accumulate characters into the input buffer
      inputBuffer += receivedChar;
    }
   delay(10);
  }

  lcd.setCursor(0, 0);
  lcd.print("Unit:");
  lcd.print(total_unt);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Price:");
  lcd.print(price1);
  lcd.print("   ");

  lcd.setCursor(11, 0);
  lcd.print("Pulse");

  lcd.setCursor(13, 1);
  lcd.print(pulse);
  lcd.print("   ");

  if (total_unt == 5)
  {
    if (flag1 == 0)
    {
      flag1 = 1;
      digitalWrite(buzzer, HIGH);
      sendSMS(phone_no1, "Your Balance is Low Please Recharge");
      digitalWrite(buzzer, LOW);
    }
  }

  if (total_unt == 0)
  {
    digitalWrite(relay, LOW);
    digitalWrite(relay2, LOW);
    if (flag2 == 0)
    {
      flag2 = 1;
      digitalWrite(buzzer, HIGH);
      sendSMS(phone_no1, "Your Balance is Finish Please Recharge");
      digitalWrite(buzzer, LOW);
    }
  }
}

void load_on()
{
  Write();
  Read();
  digitalWrite(relay, HIGH);
  digitalWrite(relay2, HIGH);
  flag1 = 0, flag2 = 0;
}

void sendSMS(String number, String msg)
{
  Serial.print("AT+CMGS=\"");
  Serial.print(number);
  Serial.println("\"\r\n");
  delay(500);
  Serial.println(msg);
  delay(500);
  Serial.write(byte(26));
  delay(5000);
}

void Data()
{
  Serial.print("AT+CMGS=\"");
  Serial.print(phone_no1);
  Serial.println("\"\r\n");
  delay(1000);
  Serial.print("Unit:");
  Serial.println(total_unt);
  Serial.print("Price:");
  Serial.println(price1);
  delay(500);
  Serial.write(byte(26));
  delay(5000);
}

void Read()
{
  unt_a = EEPROM.read(1);
  unt_b = EEPROM.read(2);
  unt_c = EEPROM.read(3);
  unt_d = EEPROM.read(4);
  total_unt = unt_d * 1000 + unt_c * 100 + unt_b * 10 + unt_a;
  price1 = total_unt * Set;
}

void Write()
{
  unt_d = total_unt / 1000;
  total_unt = total_unt - (unt_d * 1000);
  unt_c = total_unt / 100;
  total_unt = total_unt - (unt_c * 100);
  unt_b = total_unt / 10;
  unt_a = total_unt - (unt_b * 10);

  EEPROM.write(1, unt_a);
  EEPROM.write(2, unt_b);
  EEPROM.write(3, unt_c);
  EEPROM.write(4, unt_d);
}

void initModule(String cmd, char *res, int t)
{
  Serial.print("Sending: ");
  Serial.println(cmd);

  Serial.println(cmd);
  delay(100);

  delay(t);
}

void ai0()
{
  if (digitalRead(pulse_in) == 1)
  {
    pulse = pulse + 1;
    if (pulse > 50)
    {
      pulse = 0;
      if (total_unt > 0)
      {
        total_unt = total_unt - 1;
      }
      Write();
      Read();
    }
    EEPROM.write(10, pulse);
  }
}

void processInput(String command) 
{
  // Compare the received command and take appropriate actions
  if (command == "ATrech1")
   {
    price = 100 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge is Done: 100");
                  sendSMS(phone_no2, "Your Recharge is Done: 100");
                  load_on();
  } 
  else if (command == "ATrech2") 
  {
    price = 200 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge is Done: 200");
                  sendSMS(phone_no2, "Your Recharge is Done: 200");
                  load_on();
  }
   else if (command == "ATrech3") 
  {
    price = 300 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge is Done: 300");
                  sendSMS(phone_no2, "Your Recharge is Done: 300");
                  load_on();
  }
   else if (command == "ATrech4") 
  {
     price = 400 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge is Done: 400");
                  sendSMS(phone_no2, "Your Recharge is Done: 400");
                  load_on();
  }
   else if (command == "ATrech5") 
  {
    digitalWrite(relay, LOW);
                  digitalWrite(relay2, HIGH);
                  sendSMS(phone_no1, "LOAD 1 is off");
                  sendSMS(phone_no2, "LOAD 1 is off");
                  //load_on();
  }
   else if (command == "ATrech6") 
  {
    digitalWrite(relay2, LOW);
                 digitalWrite(relay, HIGH);
                  sendSMS(phone_no1, "LOAD 2 is off");
                  sendSMS(phone_no2, "LOAD 2 is off");
  }
  // Add more conditions for other commands if needed
}
