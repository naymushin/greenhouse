#include <OneWire.h>
#include <LiquidCrystal.h>
#include <Bounce.h>

#define ButtonModePin 1  // number of pin of button that controls mode
#define ButtonPlusPin 2  // number of pin of button that increases a value
#define ButtonMinusPin 3 // number of pin of button that decreases a value

LiquidCrystal lcd(0, 1, 2, 3, 4, 5);

int Vlazh1; // value of 1st humidity sensor
int Vlazh2; // value of 2nd humidity sensor
int Vlazh3; // value of 3rd humidity sensor

int ErrorHum = 0;  // are humidity sensors broken?: 0,3123,212,223,213,11,12,13
int ErrorTemp = 0; // are temperature sensors broken?: 0,212,11,12

const byte Vlazh1Pin = 1; // number of pin of 1st humidity sensor
const byte Vlazh2Pin = 1; // number of pin of 2nd humidity sensor
const byte Vlazh3Pin = 1; // number of pin of 3rd humidity sensor

const byte TempSensor1Pin = 1; // number of analog input of 1st temperature sensor
const byte TempSensor2Pin = 1; // number of analog input of 2nd temperature sensor

int Vlazhnost = 200; // minimum humidity level to start irrigation

int TempSensor1; // air temperature in greenhouse from 1st sensor (LM35) 
int TempSensor2; // air temperature in greenhouse from 2nd sensor (LM35)

byte TempMin = 25; // minimum temperature to start servomotors (close window and door)
byte TempMax = 35; // maximum temperature to start servomotors (open window and door)

float TempBochki; // temperature of water in a barrel from sensor (DS18B20)

int TempBochkiMin = 15; // minimum teperature of water that can be poured out
byte TempBochkiPin; // number of pin of water temperature sensor in a barrel
boolean ErrorBochki = false; // is water temperature sensor in a barrel broken?

const byte KlapanPin = 1; // number of digital output of a relay of solenoid valve
                          // that controls irrigation process
const int DlitPoliva = 60; // duration of irrigation (in seconds)

const byte ReleFrOpen = 2; // numbers of pins connected to relay for window opening,
const byte ReleFrClose = 3; // for window closing,
const byte ReleDvOpen = 4; // for door opening,
const byte ReleDvClose = 5; // for door closing

const int FrOpenCloseTime = 3; // time to partially open/close a window (in seconds)
const int DvOpenCloseTime = 4; // time to partially open/close a door (in seconds)
const unsigned int Zaderzhka = 30; // duration of cycle (check) in seconds
unsigned long TimeCheck = 0;
unsigned int raw;

byte Sost = 0; // state of input mode: 0\1\2\3\4
boolean Nachalo = true;
byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];

OneWire ds(TempBochkiPin);

Bounce Mode = Bounce(ButtonModePin, 200); // an object of bounce class (for button
                                          // that controls mode of input)
Bounce Plus = Bounce(ButtonPlusPin, 150); // an object of bounce class (for button
                                          // that increases the value)
Bounce Minus = Bounce(ButtonMinusPin, 150); // an object of bounce class (for button
                                            // that decreases the value)

// configuration of system                                          
void setup() { 
  
    // setting relays outputs for servomotors and solenoid valve to the mode
    // of digital output
    pinMode(KlapanPin, OUTPUT);
    pinMode(ReleFrOpen, OUTPUT);
    pinMode(ReleDvOpen, OUTPUT);
    pinMode(ReleFrClose, OUTPUT);
    pinMode(ReleDvClose, OUTPUT);

    // setting buttons outputs to the mode of digital output
    pinMode(ButtonModePin,INPUT);
    pinMode(ButtonPlusPin,INPUT);
    pinMode(ButtonMinusPin,INPUT);
    
    lcd.begin(16, 2);
    delay(5000); // delay for 5 seconds for stabilization of all devices
}

// main program
void loop() { 

    Sost = 0;
    ErrorBochki = false;
    ErrorHum = 0; // all humidity sensors are working properly
    ErrorTemp = 0; // all temperature sensors are working properly
    
    if (Nachalo == true){goto Schityvanie;} // if MC is just on, then go to reading
    
    Nachalo = false; // there is possibility to enter data in the next cycle
    TimeCheck = 0; // reset counter of time
    
    // perform cycle with possibility of entering data,
    // while time of check isn't expired
    while(TimeCheck < Zaderzhka * 1000) { 
    
        // considering dеlay for pin bounce,if there was click with duration of 200 ms
        // and the button for mode changing is pressed
        if (Mode.update() && Mode.read() == HIGH) { 
        
            switch (Sost) { // occurs change of menu view (mode of input)
            
                case 0: Sost = 1; // menu "changing the minimal temperature"
                
                        lcd.clear();
                        lcd.print("M""\xB8\xBD\xB8\xBC""a""\xBB\xC4"); 
                        lcd.print("\xBD""a""\xC7"" ""\x85"" ""\x84"); 
                        lcd.setCursor(0, 1);                                                           
                        lcd.print("\xBF""e""\xBC\xBE""epa""\xBF\x79""pa ");                           
                        lcd.print(TempMin);                                                           
                        lcd.print("\x99""C");                                                                       
                        break;   
                        
                case 1: Sost=2; // menu "changing the maximum temperature"
                    
                        lcd.clear();                                                                              
                        lcd.print("Ma""\xBA""c""\xB8\xBC""a""\xBB\xC4"); 
                        lcd.print("\xBD""a""\xC7"" ""\x85"" ""\x84"); 
                        lcd.setCursor(0, 1);                                                             
                        lcd.print("\xBF""e""\xBC\xBE""epa""\xBF\x79""pa ");                             
                        lcd.print(TempMax);                                                             
                        lcd.print("\x99""C");                                                               
                        break; 
                        
                case 2: Sost=3; // menu "changing the minimum humidity"
                    
                        lcd.clear();                                                             
                        lcd.print("M""\xB8\xBD\xB8\xBC""a""\xBB\xC4");   
                        lcd.print("\xBD""a""\xC7"" ""\x85"" ""\x84");   
                        lcd.setCursor(0, 1);                                                             
                        lcd.print("\xB3""\xBB""a""\xB6\xBD""oc""\xBF\xC4 ");                            
                        lcd.print(Vlazhnost);                                                           
                        lcd.print("\x25");                                                              
                        break; 
                        
                case 3: Sost=4; // menu "changing the minimum temperature in a barrel"
                    
                        lcd.clear();
                        lcd.print("M""\xB8\xBD"".""\xBF""e""\xBC\xBE"".");           
                        lcd.setCursor(0, 1);                                          
                        lcd.print("\xB3""o""\xE3\xC3"" ""\xB2""o""\xCO\xBA\xB8"" ");  
                        lcd.print(TempBochkiMin);                                    
                        lcd.print("\x99""C");                                         
                        break; 
                        
                case 4: Sost=0; // menu "current (last) values of temperature
                                // and humidity"
                        lcd.clear();
                        lcd.print("Te""\xBC\xBE""epa""\xBF\x79""pa ");      
                        lcd.print(int((TempSensor1 + TempSensor2) / 2));        
                        lcd.print("\x99""C");                               
                        lcd.setCursor(0, 1);                                 
                        lcd.print("  B""\xBB""a""\xB6\xBD""oc""\xBF\xC4 "); 
                        lcd.print(int((Vlazh1 + Vlazh2 + Vlazh3) / 30));          
                        lcd.print("\x25");                                  
                        break;                                              
            }
        }
    }
    
    // considering dеlay for pin bounce,if there was click with duration of 150 ms
    // and the ButtonPlus is pressed
    if (Plus.update() && Plus.read() == HIGH) {
        
        switch (Sost) { // occurs change of menu view (mode of input)
        
            case 1: // mode of changing the minimum temperature
            
                // if minimum temperature is less than maximum for only 0...2 degrees,
                // then minimum temperature remains unchanged
                if (TempMin > TempMax - 3){;}  
                else {
                    
                    TempMin = TempMin + 1; // else minimum temperature increases
                                           // for 1 degree
                    lcd.setCursor(12, 1); // sets cursor to enter new value of min.
                                          // temperature on lcd display          
                    lcd.print(TempMin); // prints new min. temperature
                }                      
                break;  
                
            case 2: // mode of changing the maximum temperature
    
                // if maximum temperature is greater than 79 degrees,
                // then maximum temperature remains unchanged
                if (TempMax > 79){;}                        
                else {
                    
                    TempMax = TempMax + 1; // else minimum temperature increases
                                           // for 1 degree
                    lcd.setCursor(12, 1);           
                    lcd.print(TempMax);   
                }
                break;
                
            case 3: // mode of changing the minimum humidity
            
                // if percentage of humidity is greater than 89, than minimum
                // value of humidity remains unchanged
                if (int(Vlazhnost / 10) > 89){;} 
                else {
                    
                    Vlazhnost = Vlazhnost + 10; // else minimum value of humidity
                                                // increases for 1%
                    lcd.setCursor(12,1);                
                    lcd.print(Vlazhnost);
                }
                break; 
                
            case 4: // mode of changing the minimum temperature of water in a barrel
                
                // if value of temperature is greater than 89 degrees,
                // then minimum value of temperature remains unchanged
                if (TempBochkiMin > 89){;}
                else {
                    
                    TempBochkiMin = TempBochkiMin + 1; // else minimum value of
                                                       // temperature increases
                                                       // for 1 degree
                    lcd.setCursor(12, 1);                                         
                    lcd.print(TempBochkiMin);
                }
                break;       
        } 
    } 
    
    // ButtonMinus is pressed
    if (Minus.update() && Minus.read() == HIGH) {
        
        switch (Sost) {
            
            case 1:
                if (TempMin < 1){;}
                else {
                    
                    TempMin = TempMin - 1;
                    lcd.setCursor(12, 1);                                
                    lcd.print(TempMin);
                }
                break;
                
            case 2:
                if (TempMax < TempMin + 3){;}
                else {
                    
                    TempMax = TempMax - 1;
                    lcd.setCursor(12, 1);                                
                    lcd.print(TempMax);
                }
                break;
                
            case 3:
                if (int(Vlazhnost / 10) < 6){;}
                else {
                    
                    Vlazhnost = Vlazhnost - 10;
                    lcd.setCursor(12, 1);                                
                    lcd.print(int(Vlazhnost / 10));
                }
                break; 
                
            case 4:
                if (TempBochkiMin < 1){;}                           
                else {
                    TempBochkiMin = TempBochkiMin - 1;
                    lcd.setCursor(12, 1);                                          
                    lcd.print(TempBochkiMin);                       
                }
                break;                                       
        }
    } 

    TimeCheck = TimeCheck + 10;
    delay(10);

    // Reading and processing data from sensors
    Schityvanie:
    
    lcd.clear();
    lcd.print("O""\xB2""pa""\xB2""o""\xBF""\xBA""a"); 
    lcd.setCursor(1, 1);                               
    lcd.print("\xE3""a""\xBD\xBD\xC3""x...");         
 
    Vlazh1 = analogRead(Vlazh1Pin); // reads data from 1st sensor of humidity
    Vlazh2 = analogRead(Vlazh2Pin); // reads data from 2nd sensor of humidity
    Vlazh3 = analogRead(Vlazh3Pin); // reads data from 3rd sensor of humidity

    // Processing of errors if all humidity sensors are broken
    if (Vlazh1 < 3 && Vlazh2 < 3 && Vlazh3 < 3) {
        
        ErrorHum = 3123; 
        goto Konets;
    }

    // Processing of errors if two humidity sensors are broken
    if (Vlazh1 < 3 && Vlazh2 < 3) {
 
        Vlazh1 = Vlazh3;
        Vlazh2 = Vlazh3;
        ErrorHum = 212;
        goto PropuskOshibok;
    }
    
    if (Vlazh2 < 3 && Vlazh3 < 3) {
        
        Vlazh2 = Vlazh1;
        Vlazh3 = Vlazh1;
        ErrorHum = 223;
        goto PropuskOshibok;
    }
    
    if (Vlazh1 < 3 && Vlazh3 < 3) {
        
        Vlazh1 = Vlazh2;
        Vlazh3 = Vlazh2;
        ErrorHum = 213;
        goto PropuskOshibok;
    }

    // Processing of errors if only one humidity sensor is broken
    if (Vlazh1 < 3) { // 1st humidity sensor is broken
        
        Vlazh1 = Vlazh2;
        ErrorHum = 11;
    }
 
    if (Vlazh2 < 3) {
        
        Vlazh2 = Vlazh3;
        ErrorHum = 12; // 2nd humidity sensor is broken
    }
    
    if (Vlazh3 < 3) {
        
        Vlazh3 = Vlazh2;
        ErrorHum = 13; // 3rd humidity sensor is broken
    }
    
    PropuskOshibok:

    // Reading data from sensor of water temperature in a barrel
    present = 0;
    
    if (!ds.search(addr)) {
    
        ds.reset_search();
        delay(250);
        return;
    }
    
        ds.reset();
        ds.select(addr);
        ds.write(0x44,1); // start conversion, with parasite power at the end
        delay(1000); // 1000ms should be enough

        present = ds.reset();
        ds.select(addr);    
        ds.write(0xBE); // read scratchpad
        for (i = 0; i < 9; i++) // we need 9 bytes
            data[i] = ds.read();

        // convert the data to actual temperature
        raw = (data[1] << 8) | data[0];
    
        if (type_s) {
        
            raw = raw << 3; // 9 bit resolution by default
        
            if (data[7] == 0x10) // count remain gives full 12 bit resolution       
                raw = (raw & 0xFFF0) + 12 - data[6];
            
            else {

            byte cfg = (data[4] & 0x60);
    
            if (cfg == 0x00)
                raw = raw << 3;  // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20)
                raw = raw << 2; // 10 bit res, 187.5 ms
            else if (cfg == 0x40)
                raw = raw << 1; // 11 bit res, 375 ms
            // default is 12 bit resolution, 750 ms conversion time
        }
    
        TempBochki = (float)raw / 16.0;
        TempBochki = int(TempBochki);
 
        // Reading data from water temperature sensor in a barrel is completed

        if (TempBochki < 1) {
            
            ErrorBochki = true;
            goto Konets;
        }

        // if mean value of humidity based on three humidity sensors is less than standard
        // and temperature of water in a barrel is enough high, then irrigation starts
        if (((Vlazh1 + Vlazh2 + Vlazh3) / 3) < Vlazhnost && TempBochki >= TempBochkiMin) { 

            digitalWrite(KlapanPin, HIGH); // valve is opened (relay is triggered)
            delay(DlitPoliva * 1000); // water flows several seconds until
            digitalWrite(KlapanPin, LOW); // valve isn't closed (relay is untriggered)
        } else
            goto VlazhVysoka; // humidity is too high or water in a barrel is too cold
      
        VlazhVysoka:
        
        // opening window and door
        TempSensor1 = analogRead(TempSensor1Pin);
        TempSensor1 = (500 * TempSensor1) >> 10;
        TempSensor2 = analogRead(TempSensor2Pin);
        TempSensor2 = (500 * TempSensor2) >> 10;
    
        if (TempSensor1 < 0 && TempSensor2 < 0) {
            
            ErrorTemp = 212;
            goto Konets;
        }
    
        if (TempSensor1 < 0) {
            
            TempSensor1 = TempSensor2;
            ErrorTemp = 11;
        }
    
        if (TempSensor2 < 0) {
            
            TempSensor2 = TempSensor1;
            ErrorTemp = 12;
        }
    
        // if average temperature from sensors is greater than
        // maximum allowable in greenhouse, then
        if ((TempSensor1 + TempSensor2) / 2 > TempMax) { 
            // open a window
            digitalWrite(ReleFrOpen, HIGH); // relay is triggered, 
                                            // servomotor starts rotating
                                            // and window is opened
            delay(FrOpenCloseTime * 1000); // servomotor works for several seconds, then
            digitalWrite(ReleFrOpen, LOW); // realy is untriggered (servomotor stops)

            // open a door
            digitalWrite(ReleDvOpen, HIGH);   
            delay(DvOpenCloseTime * 1000);   
            digitalWrite(ReleDvOpen, LOW);

            goto Konets;
        }
    
        // close a window
        if ((TempSensor1 + TempSensor2) / 2 < TempMin) {  

            digitalWrite(ReleFrClose, HIGH);                              
            delay(FrOpenCloseTime * 1000);                                
            digitalWrite(ReleFrClose LOW);                               
        
            // close a door
            digitalWrite(ReleDvClose, HIGH);     
            delay(DvOpenCloseTime * 1000);  
            digitalWrite(ReleDvClose, LOW);   
        }
    
        Konets:
        
        lcd.clear();
    
        switch (ErrorTemp) {
            
            case 0:
                lcd.print("Te""\xBC\xBE""epa""\xBF\x79""pa ");      
                lcd.print(int((TempSensor1 + TempSensor2) / 2));
                lcd.print("\x99""C");  
                break;  
            
            case 212:
                lcd.print("Temp Error:12");
                break;
        
            case 11:
                lcd.print("Temp Error:1");
                break;
            case 12:
                lcd.print("Temp Error:2");
                break;
        }
      
        lcd.setCursor(0, 1); 
    
        switch (ErrorHum) {
            
            case 0: // all sensors are working properly
                lcd.print("  B""\xBB""a""\xB6\xBD""oc""\xBF\xC4 ");
                lcd.print(int((Vlazh1+Vlazh2+Vlazh3)/30));    
                lcd.print("\x25");                            
                break;
            
            case 3123: // all sensors of humidity are broken
                lcd.print("Vlazh Error:123");
                break;
        
            case 212: // 1st and 2nd sensors of humidity are broken
                lcd.print("Vlazh Error:12");
                break;
                
            case 223: // 2nd and 3rd sensors of humidity are broken
                lcd.print("Vlazh Error:23");
                break;

            case 213: // 1st and 3rd sensors of humidity are broken
                lcd.print("Vlazh Error:13");
                break;

            case 11: // 1st sensor of humidity is broken
                lcd.print("Vlazh Error:1");
                break;
       
            case 12: // 2nd sensor of humidity is broken
                lcd.print("Vlazh Error:2");
                break;
      
            case 13: // 3rd sensor of humidity is broken
                lcd.print("Vlazh Error:3");
                break;
        }
    
        if (ErrorBochki != false) { 
        
            lcd.clear();
            lcd.print("Bochka Error");
        }
    }
}