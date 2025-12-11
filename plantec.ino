// PlanTec 2.1 - Umidade + GSM + Irrigação
// Solo seco = valor alto / Solo úmido = valor baixo
// Relé invertido: LOW = LIGA / HIGH = DESLIGA
// Irrigação ativa por 5s após SMS "SIM"

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//LCD
LiquidCrystal_I2C lcd(0x27,20,4);

//SENSOR
int sensor = A0;
int leitura;
int umidadePercentual;

//GSM
SoftwareSerial gsm(10, 11);
String telefone = "+558294022654";

//IRRIGAÇÃO
int releIrrigacao = 8;

bool smsEnviado = false;

int limite_seco = 700;
unsigned long tempoIrrigacao = 5000;
unsigned long inicioIrrigacao = 0;
bool irrigando = false;

void setup(){
  Serial.begin(9600);
  gsm.begin(9600);

  lcd.init(); 
  lcd.backlight();

  lcd.setCursor(0,0); 
  lcd.print("Indice Umidade");
  lcd.setCursor(0,2); 
  lcd.print("PlanTec - G15");

  pinMode(releIrrigacao, OUTPUT);

  digitalWrite(releIrrigacao, HIGH); 

  delay(2000);
  gsm.println("AT");
  delay(1000);
  gsm.println("AT+CMGF=1");
  delay(1000);
  gsm.println("AT+CNMI=1,2,0,0,0");
  delay(1000);
}



void sendSMS(String msg){
  gsm.println("AT+CMGF=1");
  delay(500);
  gsm.print("AT+CMGS=\"");
  gsm.print(telefone);
  gsm.println("\"");
  delay(500);
  gsm.println(msg);
  delay(500);
  gsm.write(26);
  delay(5000);
}



void receberSMS(){
  if(gsm.available()){
    String sms = gsm.readString();
    sms.toUpperCase();

    if(sms.indexOf("SIM") >= 0 && !irrigando){
      
      digitalWrite(releIrrigacao, LOW);

      irrigando = true;
      inicioIrrigacao = millis();

      sendSMS("Irrigacao ativada! Funcionando por 5 segundos.");
      Serial.println("IRRIGACAO LIGADA por SMS!");

      lcd.setCursor(0,3); 
      lcd.print("Irrigando...       ");
    }
  }
}



void loop(){
  leitura = analogRead(sensor);

  umidadePercentual = map(leitura, 0, 1200, 100, 0);

  Serial.print("Bruto: ");
  Serial.print(leitura);
  Serial.print(" | Umidade: ");
  Serial.print(umidadePercentual);
  Serial.println("%");

  lcd.setCursor(5,1);
  lcd.print("       ");
  lcd.setCursor(5,1);
  lcd.print(umidadePercentual);
  lcd.print("%");

if (umidadePercentual <= 30 && !smsEnviado) {
    sendSMS("ALERTA: SOLO SECO!\nUmidade: " + String(umidadePercentual) +
            "%\nResponder: SIM para ativar irrigacao.");
    smsEnviado = true;
    Serial.print("Enviando Mensagem de Alerta");
}

if (umidadePercentual > 30 && smsEnviado) {
    smsEnviado = false;
    sendSMS("Solo umido novamente! (" + String(umidadePercentual) + "%)");
    Serial.print("Enviando mensagem de regularização");

    digitalWrite(releIrrigacao, HIGH);

    lcd.setCursor(0,3); 
    lcd.print("Sistema estavel.   ");
}

  receberSMS();

  delay(800);

  if(irrigando && millis() - inicioIrrigacao >= tempoIrrigacao){

    irrigando = false;

    digitalWrite(releIrrigacao, HIGH);

    sendSMS("Irrigacao finalizada automaticamente (5s).");
    Serial.println("IRRIGACAO DESLIGADA - TEMPO FINALIZADO");

    lcd.setCursor(0,3); 
    lcd.print("Irrigacao concluida");
  }
}
