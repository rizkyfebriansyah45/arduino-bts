#include <SPI.h> // Ethernet Start
#include <Ethernet.h>
#include <PubSubClient.h>

const char* mqtt_server = "202.65.123.134";

// Connect an LED to each GPIO of your ESP8266
const int RELAY1 = 7;
const int RELAY2 = 8; // Pin to Relay (D1 on board)

EthernetClient esp_bts1_Client;
PubSubClient client(esp_bts1_Client);

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
}; // Ethernet end

int analogInput = A4;
int analogInput2 = A3; // Volt Sensor 2 (Charger)
float vout = 0.0;
float vin = 0.0;
float R1 = 30000.0; //  
float R2 = 7500.0; // 

float voltAcc = 0.95;
float batteryAh = 12.0;
float eff = 0.9;

int value = 0;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

void callback(char* topic, byte* message, unsigned int length) {
  String strTopic = String ((char*)topic);
Serial.print("Message arrived on topic: ");
Serial.print(strTopic);
Serial.print(". Message: ");
String messageTemp;
 
for (int i=0;i < length;i++)
{ Serial.print((char)message[i]);
messageTemp += (char)message[i];
} Serial.println();

// Feel free to add more if statements to control more GPIOs with MQTT
// If a message is received on the topic dc/dc1/gpio2, you check if the message is either 1 or 0. Turns the ESP GPIO according to the message
if(strTopic=="bts/bts1/relay1"){ 
  Serial.print("Changing RELAY 1 to ");
if(messageTemp == "1")
{ 
digitalWrite(RELAY1, HIGH);
Serial.print("On"); 
}
else if(messageTemp == "0")
{ 
digitalWrite(RELAY1, LOW);
Serial.print("Off"); 
} 
}
if(strTopic=="bts/bts1/relay2")
{ Serial.print("Changing RELAY 2 to ");
if(messageTemp == "1")
{ digitalWrite(RELAY2, HIGH);
Serial.print("On"); }
else if(messageTemp == "0")
{ digitalWrite(RELAY2, LOW);
Serial.print("Off");
}
}
Serial.println(); }

void reconnect() { // Loop until we're reconnected
  while (!client.connected()) { 
    Serial.print("Attempting MQTT connection..."); // Attempt to connect
  if (client.connect("ARDUINO_BTS1")) {
  Serial.println("connected"); // Subscribe or resubscribe to a topic
  // You can subscribe to more topics (to control more LEDs in this example)
  client.subscribe("bts/bts1/relay1");
  client.subscribe("bts/bts1/relay2");
  }
  else { Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000); } } }

void setup(){
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
   pinMode(analogInput, INPUT);
   pinMode(analogInput2, INPUT);
   Serial.begin(115200);
   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
   //Serial.print("DC VOLTMETER");

  // Ethernet Start
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP()); // Ethermet End
  
}
void loop(){
      // Ethernet Start
    switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  } // Ethernet End
  
  if (!client.connected()) {
   reconnect();
  }

//client.loop();  
 if(!client.loop()) {
    client.connect("ARDUINO_BTS1");
  }
  
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 5000) {
  lastMeasure = now;
  
  unsigned int x=0;
  float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;

  for (int x = 0; x < 150; x++){ //Get 150 samples
  AcsValue = analogRead(A5);     //Read current sensor values   
  Samples = Samples + AcsValue;  //Add samples together
  delay (3); // let ADC settle before next sample 3ms
  }
  
  AvgAcs = Samples / 150.0; //Taking Average of Samples
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0)) )/0.066;
  static char ampTemp[7];
  dtostrf(AcsValueF, 6, 2, ampTemp);
  
  //unsigned int y=0;
  //float AcsValue2=0.0,Samples2=0.0,AvgAcs2=0.0,AcsValueF2=0.0;

  //for (int y = 0; y < 150; y++){ //Get 150 samples
  //AcsValue2 = analogRead(A2);     //Read current sensor values   
  //Samples2 = Samples2 + AcsValue2;  //Add samples together
  //delay (3); // let ADC settle before next sample 3ms
  //}
  
  //AvgAcs2 = Samples2 / 150.0; //Taking Average of Samples
  //AcsValueF2 = (2.5 - (AvgAcs2 * (5.0 / 1024.0)) )/0.066;
  //static char ampchargerTemp[7];
  //dtostrf(AcsValueF2, 6, 2, ampchargerTemp);

  // read the value at analog input
  value = analogRead(analogInput);
  vout = (value * 5.0) / 1024.0; // see text
  float vin = vout / (R2/(R1+R2)) * voltAcc; 
  static char voltTemp[7];
  dtostrf(vin, 6, 2, voltTemp);

  // read the value at analog input
  value = analogRead(analogInput2);
  vout = (value * 5.0) / 1024.0; // see text
  float vincharger = vout / (R2/(R1+R2)) * voltAcc; 
  static char voltchargerTemp[7];
  dtostrf(vincharger, 6, 2, voltchargerTemp);

  float battWatt = AcsValueF * vin;
  static char battWattTemp[7];
  dtostrf(battWatt, 6, 2, battWattTemp);

  //float chargerWatt = AcsValueF2 * vincharger;
  //static char chargerWattTemp[7];
  //dtostrf(chargerWatt, 6, 2, chargerWattTemp);

  //float watt = (AcsValueF2 * vincharger) + (AcsValueF * vin);
  float watt = AcsValueF * vin;
  static char wattTemp[7];
  dtostrf(watt, 6, 2, wattTemp);
  
  float batteryTime = (vin * batteryAh * eff / watt * 60) - (22 * batteryAh * eff / watt * 60);
  static char batteryTimeTemp[7];
  dtostrf(batteryTime, 6, 2, batteryTimeTemp);

  client.publish("bts/bts1/voltcharger", voltchargerTemp);
  client.publish("bts/bts1/ampcharger", ampchargerTemp);
  client.publish("bts/bts1/amp", ampTemp);
  client.publish("bts/bts1/volt", voltTemp);
  client.publish("bts/bts1/battwatt", battWattTemp);
  //client.publish("bts/bts1/chargerwatt", chargerWattTemp);
  client.publish("bts/bts1/watt", wattTemp);
  client.publish("bts/bts1/batteryTime", batteryTimeTemp);

//Serial.print("Charger Volt= ");
//Serial.print(voltchargerTemp);
//Serial.print("\t" "Charger Amps: ");
//Serial.print(ampchargerTemp);
//Serial.print("\t" "Voltage= ");
//Serial.print(voltTemp);
//Serial.print("\t" "Amps: ");
//Serial.print(ampTemp);//Print the read current on Serial monitor
//Serial.print("\t" "Charger Watts: ");
//Serial.print(chargerWattTemp);
//Serial.print("\t" "Battery Watts: ");
//Serial.print(battWattTemp);
//Serial.print("\t" "Total Watts: ");
//Serial.print(wattTemp);
//Serial.print("\t" "Battery Time: ");
//Serial.print(batteryTimeTemp);
//Serial.println(" minutes");
//delay(500);
  }
}
