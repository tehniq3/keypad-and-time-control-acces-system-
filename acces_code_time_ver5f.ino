// sketch acces bloc cu cod din exterior si buton temperizat din interior
// versiune 1.0 - 31.08.2016, Craiova, Romania
// autor: Nicu FLORICA (niq_ro) - http://www.tehnic.go.ro
// http://nicuflorica.blogspot.ro/
// http://arduinotehniq.blogspot.com/
// http://www.arduinotehniq.com/
// versiune 5 - bloacare automata noapte, deblocare automata zi si manual
// versiune 5a - adaugat ceas animat
// versiune 5b - adaugat a 2-a parola
// versiune 5c - parola a 2-a e functionala doar daca sistemul e dezactivat
// versiune 5d - finisaje
// versiune 5e - schimbat simboluri de ceas (renuntat la versiune)
// versiune 5f - afisare urmatoare inchidere/deschidere programata

#include <Wire.h>                // folosire librarie pentru protocol i2c
#include <LiquidCrystal_I2C.h>   // folosire librarie pentru afisaj lcd1602 pe i2c
#include "RTClib.h"
#include <Keypad.h>                // library for keyboard http://playground.arduino.cc/uploads/Code/keypad.zip
#include <Password.h>              // library for password http://playground.arduino.cc/uploads/Code/Password.zip
Password password = Password( "1234" );  // password
Password password1 = Password( "4321" );  // password

// afisaj 1602 pe i2c cu adresa 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);
// modul RTC cu DS1307 sau DS3231
RTC_DS1307 RTC;

const byte rows = 4;                     // four rows       
const byte cols = 3;                     // three columns
char keys[rows][cols] = {                // keys on keypad
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};
// byte rowPins[rows] = {R0, R1, R2, R3};
//byte colPins[cols] = {C0, C1, C2};
// tastatura plata (flat) - Adafruit type
//byte rowPins[rows] = {1, 2, 3, 4};
//byte colPins[cols] = {5, 6, 7};
// tastatura cu butoane in relief - model KB304-PAW
byte rowPins[rows] = {2, 7, 6, 4};
byte colPins[cols] = {3, 1, 5};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);


#define buton 12    // buton pus la D12
#define sens1 10
#define sens2 9
#define asteptare 8
#define mastercod 11
#define scurt 500   // timp in ms pentru eliminare 
#define tpactuator 700  // timp actionare actuatoare


// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte lacat[8] = {
  B01110,
  B10001,
  B00001,
  B00001,
  B11111,
  B11111,
  B11111,
};

byte cheie[8] = {
  B00110,
  B01001,
  B00110,
  B00010,
  B01110,
  B00010,
  B01110,
};

byte ceas1[8] = {
  B00000,
  B01110,
  B10101,
  B10101,
  B10001,
  B01110,
  B00000,
};

byte ceas2[8] = {
  B00000,
  B01110,
  B10001,
  B10111,
  B10001,
  B01110,
  B00000,
};

byte ceas3[8] = {
  B00000,
  B01110,
  B10001,
  B10101,
  B10101,
  B01110,
  B00000,
};

byte ceas4[8] = {
  B00000,
  B01110,
  B10001,
  B11101,
  B10001,
  B01110,
  B00000,
};


byte inchora = 21;  // autoclose hours
byte inchmin = 10;  // autocloese minutes
byte deschora = 7; // autoopen hours
byte deschmin = 30; // autopen minutes

void setup()
{
keypad.addEventListener(keypadEvent); // se creaza un obiect pentru urmarire apasare taste  
  
 pinMode(buton, INPUT);
 pinMode(asteptare, OUTPUT);
 pinMode(sens1, OUTPUT);
 pinMode(sens2, OUTPUT);
 pinMode(mastercod, OUTPUT);
 
 digitalWrite(buton, HIGH); // intrarea butonului in 1 logic
 digitalWrite(asteptare, HIGH);
 digitalWrite(sens1, LOW); 
 digitalWrite(sens2, LOW);
 digitalWrite(mastercod, LOW); 


lcd.begin();  // initializare afisaj
lcd.backlight(); // aprindere fundal
lcd.createChar(0, lacat); // creez caracter
lcd.createChar(1, cheie); // creez caracter
lcd.createChar(2, ceas1); // creez caracter
lcd.createChar(3, ceas2); // creez caracter
lcd.createChar(4, ceas3); // creez caracter
lcd.createChar(5, ceas4); // creez caracter
//lcd.write(byte(0)); // comanda pentru caracter
  
lcd.setCursor(0, 0);   // pozitionare cursor pe colana a 1-a din stanga - sus
lcd.print("Sistem acces cod"); // afisare text
lcd.setCursor(0, 1);   // mutare cursor pe coloana a 1-a din stanga - jos
lcd.print("ver.5f by niq_ro"); // afisare text

delay(3000);  
lcd.clear();

lcd.setCursor(0, 0);   // pozitionare cursor pe colana a 1-a din stanga - sus
//lcd.print("Usa incuiata.."); // afisare text
//lcd.write(byte(0));
lcd.write(byte(1));

  Wire.begin();
  
// part code from http://tronixstuff.wordpress.com/
Wire.beginTransmission(0x68);
Wire.write(0x07); // move pointer to SQW address
Wire.write(0x10); // sends 0x10 (hex) 00010000 (binary) to control register - turns on square wave
Wire.endTransmission();
// end part code from http://tronixstuff.wordpress.com/

    RTC.begin();
// if you need set clock... just remove // from line above this
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // RTC.adjust(DateTime(2014, 1, 21, 3, 0, 0));  
    }
// RTC.adjust(DateTime(__DATE__, __TIME__));
//RTC.adjust(DateTime(2016, 9, 11, 13, 24, 0)); 
}

void loop() {

DateTime now = RTC.now();  
   lcd.setCursor(3, 0);
   if ( now.hour() < 10)
   {
     lcd.print(" "); 
     lcd.print(now.hour(), DEC);
   }
   else
   {
   lcd.print(now.hour(), DEC);
   }
//   lcd.print(":");
if ( now.second()%2 == 0) lcd.print(":"); else lcd.print(" ");
   if ( now.minute() < 10)
   {
     lcd.print("0"); 
     lcd.print(now.minute(), DEC);
   }
   else
   {
   lcd.print(now.minute(), DEC);
   }
/*
   lcd.print(":");
   if ( now.second() < 10)
   {
     lcd.print("0"); 
     lcd.print(now.second(), DEC);
   }
   else
   {
   lcd.print(now.second(), DEC);
   }
 //  lcd.print(" "); 
*/
if (digitalRead(mastercod) == HIGH) {  // clock activated 
lcd.setCursor(10, 0);
lcd.write(byte(1));  // cheie
if (inchora <10) lcd.print(" ");
lcd.print(inchora);
lcd.print(":");
if (inchmin <10) lcd.print("0");
lcd.print(inchmin);
lcd.setCursor(10, 1);
lcd.write(byte(0));  // lacat deschis
if (deschora <10) lcd.print(" ");
lcd.print(deschora);
lcd.print(":");
if (deschmin <10) lcd.print("0");
lcd.print(deschmin);

/*
int cifra = now.second();
lcd.setCursor(9, 0);
if (cifra %4 == 0) 
{
lcd.write(byte(2));
}  
if (cifra %4 == 1) 
{
  lcd.write(byte(3));
}  
if (cifra %4 == 2) 
{
  lcd.write(byte(4));
}  
if (cifra %4 == 3) 
{
  lcd.write(byte(5));
}
*/
}
else
{
//   lcd.print(" ");
lcd.setCursor(10, 0);
lcd.print("      ");
lcd.setCursor(10, 1);
lcd.print("      ");
}

  lcd.setCursor(0, 1);
   if ( now.day() < 10)
   {
     lcd.print("0"); 
     lcd.print(now.day(), DEC);
   }
   else
   {
   lcd.print(now.day(), DEC);
   }
   lcd.print("/");
   if ( now.month() < 10)
   {
     lcd.print("0"); 
     lcd.print(now.month(), DEC);
   }
   else
   {
   lcd.print(now.month(), DEC);
   }
   lcd.print("/");
   lcd.print(now.year()-2000, DEC);
   lcd.print(""); 


keypad.getKey();
 
if (digitalRead(buton) == LOW)
{    // se apasa butonul
//actiune();
deschidere();
}

if (digitalRead(mastercod) == HIGH)   // if clock is dezactivate
{
if ((now.hour() == deschora) && now.minute() == deschmin) 
{ 
//if (digitalRead(asteptare) == HIGH) deschidere();
if ((digitalRead(asteptare) == HIGH) && (now.second()<3)) deschidere();
}
if ((now.hour() == inchora) && now.minute() == inchmin) 
{
//if (digitalRead(asteptare) == LOW) inchidere();
if ((digitalRead(asteptare) == LOW) && (now.second()<3) == LOW) inchidere();
}
}


} // se termina programul


//take care of some special events
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
    case PRESSED:
//	Serial.print("Pressed: ");
//	Serial.println(eKey);
	switch (eKey){
	  case '#': checkPassword(); break;
	  case '*': password.reset(); password1.reset(); break;
	  default: password.append(eKey); password1.append(eKey);
     }
  }
}

  void checkPassword(){
  if (password.evaluate()){       // base code
//    Serial.println("Success");
     //Add code to run if it works
  if (digitalRead(asteptare) == LOW) inchidere(); else deschidere();
  password.reset();
  password1.reset();
  }
  else   if (password1.evaluate()){
//    Serial.println("Success");
     //Add code to run if it works
//  if (digitalRead(mastercod) == LOW) digitalWrite(mastercod, HIGH); else digitalWrite(mastercod, LOW);
/*
	  if ((digitalRead(asteptare) == LOW) && (digitalRead(mastercod) == HIGH)) digitalWrite(mastercod, LOW);
    else digitalWrite(mastercod, HIGH);
*/
  if (digitalRead(asteptare) == LOW)
   {
     if (digitalRead(mastercod) == HIGH) digitalWrite(mastercod, LOW);
     else digitalWrite(mastercod, HIGH);
   }    
  password.reset();
  password1.reset();
   }else{
//    Serial.println("Wrong");
    //add code to run if it did not work
    password.reset();
    password1.reset();
    delay(1000);
  }
}

void deschidere()
{
//lcd.clear();  
lcd.setCursor(0, 0);   // pozitionare cursor pe colana a 1-a din stanga - sus
lcd.write(byte(0));
//lcd.write(byte(1));
digitalWrite(sens1, HIGH);
delay(tpactuator);
digitalWrite(sens1, LOW);
digitalWrite(asteptare, LOW);
//lcd.clear();  
  
}

void inchidere()
{
digitalWrite(sens2, HIGH);
delay(tpactuator);
digitalWrite(sens2, LOW);
lcd.setCursor(0, 0);   // pozitionare cursor pe colana a 1-a din stanga - sus
lcd.write(byte(1));
digitalWrite(asteptare, HIGH);
}
