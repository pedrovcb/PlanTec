//PlanTec
// Contém funcionalidades de: Sensor Umidade; LCD

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int ldr = A0;
int leitura;
int limpar = 0;

void setup()
{
  Serial.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Indice Humildade: ");
   lcd.setCursor(0,2);
  lcd.print("PlanTec");
   lcd.setCursor(2,3);
  lcd.print("G15");
}


void loop()
{
  leitura = analogRead(ldr);
  Serial.println(leitura); 
  lcd.setCursor(5,1);
  lcd.print("    ");
  lcd.setCursor(5,1);
  lcd.print(leitura);
  delay(1000);
}
