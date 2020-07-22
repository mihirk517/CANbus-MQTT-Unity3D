#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define slaveAddress 9
float Position;
static char POSITION[5];


const char* ssid = "SSID";
const char* password = "Password";
const char* mqtt_server = "192.168.178.80";
int currentState;
String response;
WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");    
  }
  
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(String topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String command;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    command += (char)payload[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="motor/commands"){
      Serial.print("Motor Command Recieved :");
      if(command == "LOAD"){Serial.println("Load");currentState=2;}
      if(command == "UNLOAD"){Serial.println("Unoad");currentState=3;}
      if(command == "ESTOP"){Serial.println("Emergency");currentState=4;}
      Wire.beginTransmission(slaveAddress);
      Wire.write(currentState);
      Wire.endTransmission();
      Serial.println(currentState);
      
  }
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
      if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("motor/commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
      Serial.begin(115200);
      Serial.print("ESP Ready");
      Wire.begin();
      pinMode(LED_BUILTIN,OUTPUT);
      setup_wifi();
      client.setServer(mqtt_server, 1883);
      client.setCallback(callback);  
}
void loop()
{        
  if (!client.connected()) {
    digitalWrite(LED_BUILTIN,LOW);
                    reconnect();
     }
              
  if(!client.loop()){
          client.connect("ESP8266Client");
    }
          digitalWrite(LED_BUILTIN,HIGH);
          client.subscribe("motor/commands");          
          Wire.requestFrom(slaveAddress,5);
          String response="";
          while (Wire.available()){
            char b =Wire.read();
            response += b;           
          }
          Serial.println(response);
          Position = response.toFloat();
          dtostrf(Position, 5, 2, POSITION);
          client.publish("motor/Position", POSITION);        
}
