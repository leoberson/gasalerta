#include <ESP8266WiFi.h>// imp. da biblioteca que faz a conexao WI-FI
#include <FirebaseArduino.h> //imp. da biblioteca que faz a integracao com Firebase
#include <NTPClient.h> //imp. da biblioteca de data e hora da internet
#include <WiFiUdp.h> //imp. da biblioteca que abre uma conexao UDP com o servidor de data/hora

/// var hora 
WiFiUDP ntpUDP;

int16_t utc = -3; //UTC -3:00 Brazil
uint32_t currentMillis = 0; 
uint32_t previousMillis = 0;

//definicao das configuracoes do servidor de data e hora
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);

WiFiClient client;

//defenicao do servidor de Notificacao do Firebase
const char* host = "fcm.googleapis.com";

// Definicoes do pino ligado ao sensor 
int pin_a0 = A0;

int nivel_sensor = 75;

// defenicao da URL e codigo de atenticacao do firebase
#define firebaseURL  "gasalerta-f835c.firebaseio.com" 
#define authCode  "sBN4CW9Ea30HDUjoXBYAV4x8aRslQ4XE7we"

// defenição das configuracoes de conexao da internet
#define wifiNome "NodeMCU"
#define wifiSenha "nodemcu1"

// var que armazena quantidade de vezes de vazamento
int vVazando = 0;

//fun. de inicializacao das configuracoes do firebase
void setupFirebase(){
  Firebase.begin(firebaseURL, authCode);
  }

//fun. de configuração da conexão Wi-Fi
void setupWifi(){
 // connect to wifi.  
  WiFi.begin(wifiNome, wifiSenha);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

// fun. default. Inicializa  pinos, wifi, firebase, e servidor NTP
void setup()
{
    
  pinMode(pin_a0, INPUT);
  Serial.begin(115200);
  setupWifi();
  setupFirebase();
  timeClient.begin();
  timeClient.update();

}


//fun. de configuracao de envio de notificacao
void sendNotification() {
    String data = "{" ;
    data = data + "\"to\": \"e4ErWrwbnns:APA91bEgjZ7TOxj4SsWw5kOQilBjIzi2xGyutftKOhX0WAPxmhPd9oEbGRJhpzvIAqDQ6IbEpRNjhDmsqPDMmUH4rmm58PtsbiezbynnYtUwafXPN_ynP0Pyt_kZgmsMCBlIvhfKC7yz\"," ; // aqui deve ser colocado smartphone_id
    data = data + "\"notification\": {" ;
    data = data + "\"body\": \"Vazando Gás\"," ; //mensagem da notificacao 
    data = data + "\"title\" : \"GasAlerta\" " ; //titulo da notificacao
    data = data + "} }" ;
    Serial.println("Send data...");

    if (client.connect(host, 80)) {
        Serial.println("Connected to the server..");
        client.println("POST /fcm/send HTTP/1.1");
        client.println("Authorization: key=AAAAQufP8VA:APA91bFyRJ4h5dXdsV93E3wilZnbgsEi3aI5AcqyZ7dCJ9vAU2TddnEPuHsjTueHdUi1ArDWiMfv8ih5AdA-lFibF7LtRLYnxSE-Xk-EWYfZWZC53cUYgQWpNYnImjj9WkT91_oM8ZSO"); // aqui deve ser colocado o token de segurança do projeto Firebase
        client.println("Content-Type: application/json");
        client.println("Host: fcm.googleapis.com");
        client.print("Content-Length: ");
        client.println(data.length());
        client.print("\n");
        client.print(data);
    }
    Serial.println("Data sent...Reading response..");
    while (client.available()) {
        char c = client.read();
        Serial.print(c);
    }

    Serial.println("Finished!");
    client.flush();
    client.stop();

}

//Configuracoes de data e hora....................
#define LEAP_YEAR(_year) ((_year%4)==0)
static  byte monthDays[] = {31, 28, 31, 30 , 31, 30, 31, 31, 30, 31, 30, 31};
 
void localTime(unsigned long *timep, byte *psec, byte *pmin, byte *phour, byte *pday, byte *pwday, byte *pmonth, byte *pyear) {
  unsigned long long epoch =* timep;
  byte year;
  byte month, monthLength;
  unsigned long days;
   
  *psec  =  epoch % 60;
  epoch  /= 60; // now it is minutes
  *pmin  =  epoch % 60;
  epoch  /= 60; // now it is hours
  *phour =  epoch % 24;
  epoch  /= 24; // now it is days
  *pwday =  (epoch+4) % 7;
   
  year = 70;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) { year++; }
  *pyear=year; // *pyear is returned as years from 1900
   
  days  -= LEAP_YEAR(year) ? 366 : 365;
  epoch -= days; // now it is days in this year, starting at 0
   
  for (month=0; month<12; month++) {
    monthLength = ( (month==1) && LEAP_YEAR(year) ) ? 29 : monthDays[month];  // month==1 -> fevereiro
    if (epoch >= monthLength) { epoch -= monthLength; } else { break; }
  }
   
  *pmonth = month;  // jan eh mes 0
  *pday   = epoch+1;  // dia do mes
}
 
 
/*********FUNCOES FORMATAR DATA/HORA **************/
String zero(int a){ if(a>=10) {return (String)a+"";} else { return "0"+(String)a;} }
 
String data(){
  byte ano, mes, dia, dia_semana, hora, minuto, segundo;  
  currentMillis = millis();//Tempo atual em ms
  //Lógica de verificação do tempo
  if (currentMillis - previousMillis > 1000) {
    previousMillis = currentMillis;    // Salva o tempo atual
    unsigned long epoch =  timeClient.getEpochTime();
    localTime(&epoch, &segundo, &minuto, &hora, &dia, &dia_semana, &mes, &ano);
    return zero(dia) + "/" + zero(mes+1) + "/" + (ano+1900) + " " + zero(hora) + ":" + zero(minuto) + ":" + zero(segundo);
  }
}
/****************************** Fim funcao ***************************************/

//fun. de repetição
void loop()
{
 
  // Le os dados do pino analogico A0 do sensor
  int valor_analogico = analogRead(pin_a0);

  //verifica se está conecatdo ao wifi
  if(WiFi.status() != WL_CONNECTED){
    setupWifi();
  } else {  
      // Verifica a conexao com o firebase 
      if(Firebase.failed()){
          Serial.println(Firebase.error());
      } else {
          
        
          // Verifica o nivel de gas detectado
          if (valor_analogico > nivel_sensor)
          {
             //configuracoes para o armazennamento do hitorico de dados
             StaticJsonBuffer<200> jsonBuffer;
             JsonObject& root = jsonBuffer.createObject();
             root["status"] = true;
             root["data|hora"] = data();
             Firebase.push("historico", root);
             //armazena o status atual
             Firebase.set("MQ-6/status", true); // eh preenchido como true na base de dados do Firebase caso detecte gas
             Firebase.setString("MQ-6/data|hora", data());                          
             // se a quantidade de vez que detectou gas menor que envia notificacao
             if(vVazando < 1) {
                sendNotification(); // envia nonificacao caso detecte gas
             } 
             vVazando++; //incrementa a quantidade de vez que detectou gas
          } else {
             //configuracoes para o armazennamento do hitorico de dados
             StaticJsonBuffer<200> jsonBuffer;
             JsonObject& root = jsonBuffer.createObject();
             root["status"] = false;
             root["data|hora"] = data();
             String name = Firebase.push("historico", root);
             //armazena o status atual
             Firebase.set("MQ-6/status", false); // eh preenchido como false na base de dados do Firebase caso nao detecte gas
             Firebase.setString("MQ-6/data|hora", data());         
             vVazando = 0; // quando o status volta ao normal o valor e setado com zero
          }                  
      }  
  }   
  delay(5000); //tempo de repeticao a cada 5 segundos
}
