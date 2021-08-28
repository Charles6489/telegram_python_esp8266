#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "Musica.h"
//#define pinAlarma 13
//#define pinBoton 16
// Replace with your network credentials
const char* ssid = "Ambafiber Orozco";
const char* password = "0939979649";
const char* mqtt_server = "34.125.125.114";
//float sensorValue;
float sensor2 = 0;

// Initialize Telegram BOT
#define BOTtoken "1909985851:AAGz-B7Zq_RkcSstrYzF2ubRdEJ4HV1j8UE"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "1709607424"
#define BUTTON 0

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
long lastMsg = 0;
char msg[50];
int value = 0;
WiFiClientSecure client;

WiFiClient espClient;
PubSubClient client1(espClient);
UniversalTelegramBot bot(BOTtoken, client);
int frecmin = 1000;
int frecmax = 4000;
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
const int led = 12; 
const int llamar = 13; 
const int ledPin = 2;
const int ledOpen = 4;
const int pinBoton = 16;
bool ledState = LOW;
int pinAlarma = 5; // pin del parlante
int i; 
const int movimSensor = 14;
const int timbre = 12;// Pin para el sensor de movimiento
bool motionDetected = false;
bool timbre1 = false; 
Musica musica(pinAlarma);// desde el principio la alarma debe estar en falso para evitar que suene al Iniciar el circuito

// si se detecta un movimiento es llamada esta parte del codigo. 
ICACHE_RAM_ATTR void detectaMovimiento() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
  digitalWrite(led, 1);

  //Se activa la alarma
  for (i = frecmin; i <= frecmax; i++){
    tone(pinAlarma, i); 
  }
  for (i = frecmax; i >= frecmin; i--){
    tone(pinAlarma, i);
  }
}

String getReadings(){
  float temperature, humidity;
  temperature = (analogRead(A0)* 285.0) / 1024.0;
  //temperature = bme.readTempF();
  humidity = (analogRead(A0)* 285.0) / 1024.0;
  String message = "Temperature: " + String(temperature) + " ºC \n";
  message += "Humidity: " + String (humidity) + " % \n";
  return message;
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/abrir") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;   
      
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/cerrar") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }

   

   
     if (text == "/llamar") {
       
      digitalWrite(llamar, HIGH);
      delay(5000);
      digitalWrite(llamar, LOW);
      delay(2000);
      bot.sendMessage(chat_id,"Llamada Iniciada", "");
    
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }

   
  }
}

void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
pinMode(movimSensor, INPUT_PULLUP);
  // establecemos el pin movimSensor como interrupcion, asignamos la interrupcion en modo RISING 
  attachInterrupt(digitalPinToInterrupt(movimSensor), detectaMovimiento, RISING);
  pinMode(ledPin, OUTPUT);
    pinMode(llamar, OUTPUT);
    pinMode(pinAlarma,OUTPUT);
  pinMode(pinBoton,INPUT);

  digitalWrite(ledPin, ledState);
  pinMode(BUTTON, INPUT_PULLUP);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  client1.setServer(mqtt_server, 1883);
  client1.setCallback(callback);
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot started up", "");

}
// Variables will change:
int ledState1 = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;
void loop() {

if (!client.connected()) {
    reconnect();
  }
  client1.loop();


  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

   if(motionDetected){
    bot.sendMessage(CHAT_ID, "Se Detecto un Movimiento, Alarma Activa!!", "");
    Serial.println("Movimiento Detectado");
    motionDetected = false;
    digitalWrite(led, 0);

    //Alarma apagada
    noTone(pinAlarma);
  }

  
  int reading = digitalRead(pinBoton);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (reading == HIGH) {
        
        String welcome = "Hola el bienvenido a tu sistema de alarma  que deseas realizar\n";
      welcome += "/foto :Tomar una foto\n";
      welcome += "/abrir :Abrir la puerta\n";
      welcome += "/cerrar :Abrir la puerta\n";
      welcome += "/flash :toggle flash LED\n";
      welcome += "/video :Camara en vivo\n";
      welcome += "/alarma :Activar alarma\n";
      welcome += "/parar : Apagar alarma\n";
      welcome += "/imagenes : Imaganes Guardadas\n\n";
      welcome += "/llamar : Llamada telefono IP\n\n";
      welcome += "/temperatura : Leer temperatura\n\n";
      welcome += "Recibirás una foto cada vez que se detecte movimiento.\n";
      bot.sendMessage(CHAT_ID, welcome, "Markdown");
        musica.reproducir();
        //tiempoTranscurrido = 0;        
      } else {
        digitalWrite(pinAlarma,LOW);
      }


    }    
  }
  lastButtonState = reading;
   
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp8266/temp") {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
      Serial.println("temperatura");
      String readings = getReadings();
      bot.sendMessage(CHAT_ID, readings, "");
    }   
  }
  else if (String(topic) == "esp8266/alarma") {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
      Serial.print("Alarma activada");
         //Se activa la alarma
  for (i = frecmin; i <= frecmax; i++){
    tone(pinAlarma, i); 
  }
  for (i = frecmax; i >= frecmin; i--){
    tone(pinAlarma, i);
  }
      bot.sendMessage(CHAT_ID, "Alarma Encendida", "");
    }   
  }
  else if (String(topic) == "esp8266/apagar") {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
      Serial.print("Alarma apagada");
         noTone(pinAlarma);
      bot.sendMessage(CHAT_ID, "Alarma Apagada", "");
    }   
  }
  else if (String(topic) == "esp32/timbre") {
    Serial.print("Changing output to ");
    if(messageTemp == "true"){
     musica.reproducir();
    }   
  }
  else {}



  


  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client1.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client1.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client1.subscribe("esp32/output");
      client1.subscribe("esp8266/temp");
      client1.subscribe("esp8266/alarma");
      client1.subscribe("esp8266/apagar");
      client1.subscribe("esp8266/timbre");
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client1.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
