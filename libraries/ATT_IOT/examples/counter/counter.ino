/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2017 AllThingsTalk
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Ethernet.h>
//#include <Ethernet2.h>
#include <EthernetClient.h>

#include <PubSubClient.h>

#include <ATT_IOT.h>
#include <SPI.h>  // required to have support for signed/unsigned long type.

// define device credentials and endpoint
char deviceId[] = "";
char token[] = "";
#define httpServer "api.allthingstalk.io"  // API endpoint

ATTDevice Device(deviceId, token);           // create the object that provides the connection to the cloud to manager the device.

#define mqttServer httpServer                // MQTT Server Address 

int ledPin = 8;                                             // Pin 8 is the LED output pin + identifies the asset on the cloud platform

//required for the device
void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
  Serial.begin(9600);       // init serial link for debugging
  
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0xE1, 0x3E};  // adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                       // initialize the Ethernet connection
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);                                      // we failed to connect, halt execution here
  }
  delay(1000);  // give the Ethernet shield a second to initialize
  
  while(!Device.Connect(&ethClient, httpServer))  // connect the device with the IOT platform
    Serial.println("retrying");
    
  Device.AddAsset("counter", "counter", "counting up", "sensor", "{\"type\": \"integer\"}");
  
  while(!Device.Subscribe(pubSub))  // make certain that we can receive message from the iot platform (activate mqtt)
    Serial.println("retrying"); 
}

unsigned long time;
unsigned int prevVal = 0;
int counter = 0;
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (time + 3000))  // update and send counter value every 3 seconds
  {
    counter++;
    Device.Send(String(counter), "counter");
    time = curTime;
  }
  Device.Process(); 
}

// callback function: handles messages that were sent from the iot platform to this device
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String assetName = Device.GetAssetName(topic, strlen(topic));
  Serial.print("Data arrived from asset: ");
  Serial.println(assetName);
}