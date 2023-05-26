#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);
//RUN MQTT
//"C:\Program Files\mosquitto\mosquitto" -v
//Configure wifi
const char* ssid = "2i";
const char* password = "12341234";
//Configure MQTT
const char* mqtt_server = "192.168.43.72";
const unsigned int mqtt_port = 1883;
const char* pub_topic = "distance";
const char* sub_topic1 = "allow";
const char* sub_topic2 = "value";

boolean allow;

#define trigpin 17
#define echopin 16
#define buz 26
#define led1 25
#define led2 33
#define led3 32
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifi;
PubSubClient client(mqtt_server,mqtt_port,callback,wifi);

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message from [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length ; i++) {
    
    message += (char)payload[i];
  }
  Serial.print(message);
  Serial.println("");
  if(message == "true"){
    allow = 1;
    lcd.clear();
  }
  else if(message == "false"){
    allow = 0;
  }
  else if(message == "null"){
    lcd.setCursor(0,0);        
    lcd.print("    Waiting!    ");
    lcd.setCursor(0,1);        
    lcd.print("                ");
  }
  else{
      float value = message.toFloat();
      if(value < 2 || value > 450){
        digitalWrite(buz, LOW);
      }
      else{
        digitalWrite(buz, HIGH);
      }
      //Serial.println(value);    
      lcd.setCursor(0,0);
      lcd.print("Distance:    ");
      if(length == 4){
        lcd.setCursor(3,1);
        lcd.print("  ");
        lcd.setCursor(5,1);
        lcd.print(message);
        lcd.setCursor(9,1);
        lcd.print(" cm");
      }
      if(length == 5){
        lcd.setCursor(2,1);
        lcd.print("  ");
        lcd.setCursor(4,1);
        lcd.print(message);
        lcd.setCursor(9,1);
        lcd.print(" cm");
      }
      if(length == 6){
        lcd.setCursor(3,1);
        lcd.print(message);
        lcd.setCursor(9,1);
        lcd.print(" cm");
      }
  } 
}
unsigned long timedelay = 0;
void setup_wifi(){
  delay(100);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  lcd.clear();
  lcd.setCursor(0,0);        
  lcd.print("Connecting to... ");
  lcd.setCursor(0,1);
  lcd.print(ssid);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.setCursor(0,0);        
  lcd.print("WiFi connected");
  digitalWrite(led2,LOW);
  lcd.setCursor(0,1);
  lcd.print("IP: ");
  lcd.setCursor(3,1);
  lcd.print(WiFi.localIP());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  beep();beep();
  delay(3000);
}
float distance(){
    unsigned long duration;
    float distance;
    digitalWrite(trigpin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigpin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigpin, LOW);  

    duration = pulseIn(echopin, HIGH);
    distance = (duration/2)*0.0343;
    return distance;
}
void beep(){
  digitalWrite(buz,LOW);
  delay(200);
  digitalWrite(buz,HIGH);
  delay(100);
}
void setup() {
    Serial.begin(115200);
    pinMode(buz,OUTPUT);
    pinMode(led1,OUTPUT);
    pinMode(led2,OUTPUT);
    pinMode(led3,OUTPUT);
    digitalWrite(buz,HIGH);
    digitalWrite(led1,LOW);
    digitalWrite(led2,HIGH);
    digitalWrite(led3,HIGH);
    Wire.begin(21,22);
    lcd.begin();                
    lcd.clear();               
    lcd.backlight();
    lcd.setCursor(4,0);        
    lcd.print("Welcom!");
    beep();
    delay(1000); 
    
    setup_wifi();
    pinMode(trigpin, OUTPUT);
    pinMode(echopin, INPUT);
    
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
}
void reconnect() {
  while (!client.connected()){
    if(WiFi.isConnected()==0){
      break;
    }
    Serial.print("Connecting to MOSQUITTO MQTT...");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connecting to...");
    lcd.setCursor(4,1);
    lcd.print("Server");
    digitalWrite(led3,HIGH);
    delay(2000);
    if(client.connect("ESP32client")) {
     Serial.println("");
     Serial.println("MQTT connected");
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Server connected");
     digitalWrite(led3,LOW);
     beep();beep();
     client.subscribe(sub_topic1);
     client.subscribe(sub_topic2);
     delay(2000);
    }
    else{
      Serial.println("");
      Serial.println("MQTT connection failed ");
      Serial.println("Please try again in 3 seconds");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Server connect");
      lcd.setCursor(4,1);
      lcd.print("Failed");
      delay(3000);
    }
  }
  
}
unsigned long time1 = 0;
void loop() {
  if(WiFi.isConnected()){
    digitalWrite(led2, LOW);
    if(!client.connected()){
      reconnect();
    }
  }
  else{
    digitalWrite(led2, HIGH);
    setup_wifi();
  }
  if(allow == 1){
    char string[8];
    dtostrf(distance(), 1, 2, string);
    if((unsigned long) (millis() - time1) > 200){
    client.publish(pub_topic,string);
    time1 = millis(); 
    }
   }
  client.loop();
}
