//PlanTec - Umidade + GSM + Irrigação
// Responde SMS com [SIM] para ativar irrigação

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//---------------- LCD ----------------//
LiquidCrystal_I2C lcd(0x27,20,4);

//---------------- SENSOR -------------//
int ldr = A0;
int leitura;

//---------------- GSM ----------------//
SoftwareSerial gsm(11, 10);  // RX=11 , TX=10
String telefone = "+55XXXXXXXXXXX"; // <-- coloque seu número

//---------------- IRRIGAÇÃO ----------//
int releIrrigacao = 8;          // Saída do relé
bool smsEnviado = false;        // Controla envio único
int limite_baixo = 600;         // Umidade baixa = alerta
unsigned long tempoIrrigacao = 5000;  // 5 segundos
unsigned long inicioIrrigacao = 0;
bool irrigando = false;

//=====================================//
void setup(){
  Serial.begin(9600);
  gsm.begin(9600);

  //LCD
  lcd.init(); lcd.backlight();
  lcd.setCursor(0,0); lcd.print("Indice Umidade");
  lcd.setCursor(0,2); lcd.print("PlanTec - G15");

  //Rele irrigação
  pinMode(releIrrigacao, OUTPUT);
  digitalWrite(releIrrigacao, LOW); // começa desligado

  //Configura GSM
  delay(2000);
  gsm.println("AT");
  delay(1000);
  gsm.println("AT+CMGF=1"); // Modo texto
  delay(1000);
  gsm.println("AT+CNMI=1,2,0,0,0"); // Notifica SMS recebido automaticamente
  delay(1000);
}

//=====================================//
// ENVIO DE SMS
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

//=====================================//
// LEITURA DE SMS RECEBIDO
void receberSMS(){
  if(gsm.available()){
    String sms = gsm.readString();

    sms.toUpperCase(); // facilita detecção do SIM

    if(sms.indexOf("SIM") >= 0 && !irrigando){   // dono respondeu SIM e irrigação está desligada
		 digitalWrite(releIrrigacao, HIGH); // Ativa irrigação
		 irrigando = true;
		 inicioIrrigacao = millis();        // Marca início do tempo
		 sendSMS("Irrigacao ativada! Funcionara por 5 segundos.");
		 Serial.println("IRRIGACAO LIGADA por SMS!");
		 lcd.setCursor(0,3); lcd.print("Irrigando...       ");
		}
  }
}

//=====================================//

void loop(){
  leitura = analogRead(ldr);
  Serial.println(leitura);

  lcd.setCursor(5,1); lcd.print("    ");
  lcd.setCursor(5,1); lcd.print(leitura);

  // ----- UMIDADE BAIXA - ENVIA 1 SMS -----
  if(leitura < limite_baixo && !smsEnviado){
    sendSMS("⚠ UMIDADE BAIXA! Valor: " + String(leitura) + 
            "\nResponder: SIM para ativar irrigacao.");
    smsEnviado = true;
  }

  // ----- RESET QUANDO VOLTA AO NORMAL -----
  if(leitura >= limite_baixo && smsEnviado){
    smsEnviado = false;
    sendSMS("Umidade normalizada novamente.");
    digitalWrite(releIrrigacao, LOW);
    lcd.setCursor(0,3); lcd.print("Sistema estavel.   ");
  }

  receberSMS(); // monitora mensagens recebidas

  delay(800);
  
  // ===== CONTROLE DO TEMPO DE IRRIGAÇÃO (5s) =====
	if(irrigando && millis() - inicioIrrigacao >= tempoIrrigacao){
	 irrigando = false;
	 digitalWrite(releIrrigacao, LOW);   // Desliga irrigação
	 sendSMS("Irrigacao finalizada automaticamente (5s).");
	 Serial.println("IRRIGACAO DESLIGADA - TEMPO FINALIZADO");
	 lcd.setCursor(0,3); lcd.print("Irrigacao concluida");
	}
}
