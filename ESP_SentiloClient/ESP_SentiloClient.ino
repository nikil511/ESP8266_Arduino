/*
   Sentilo

   Copyright (C) 2013 Institut Municipal d�Inform�tica, Ajuntament de Barcelona.

   This program is licensed and may be used, modified and redistributed under the terms of the
   European Public License (EUPL), either version 1.1 or (at your option) any later version as soon
   as they are approved by the European Commission.

   Alternatively, you may redistribute and/or modify this program under the terms of the GNU Lesser
   General Public License as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   Unless required by applicable law or agreed to in writing, software distributed under the License
   is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
   or implied.

   See the licenses for the specific language governing permissions, limitations and more details.

   You should have received a copy of the EUPL1.1 and the LGPLv3 licenses along with this program;
   if not, you may find them at:

   https://joinup.ec.europa.eu/software/page/eupl/licence-eupl http://www.gnu.org/licenses/ and
   https://www.gnu.org/licenses/lgpl.txt
*/
#include <ESP8266WiFi.h>
#include <SPI.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>

#include "SentiloClient.h"

/*******************************************/
/***** SENTILO *****************************/
/*******************************************/
char* apiKey = "YOUR_API_KEY";
char* ip = "YOUR_IP_ADDRESS";
int port = 8081;
char* componentId = "sample-component";
char* providerId = "samples-provider";
char* sensorId = "sample-sensor-arduino-01";



// The Sentilo Client object
SentiloClient sentiloClient = SentiloClient(ip, port);

/*******************************************/
/***** NETWORK *****************************/
/*******************************************/
const int networkConnectionTimeout = 30;

/*******************************************/
/***** GLOBAL VARS *************************/
/*******************************************/
const int generalCalibrationTimeout = 1000; 	// Wait after system setup is complete
String response = "";            				// Rest call response (normaly as JSON message)
int statusCode = -1;                    		// Rest call return code (the HTTP code)

void setup() {
  // Begin serial for debug purposes
  Serial.begin(115200);

  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("EXM_DEVICE")) { //add chip id to make unique SSID
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
  Serial.println("connected...yeey :)");
  // Setup the Sentilo Client
  // and the network connection
  
  //setupSetiloClient();

  // Wait time for a general calibration
  delay(generalCalibrationTimeout);
}

void loop() {
  // Create the Observation object
  SentiloClient::Observation observation;
  observation.value = "666";

  Serial.println("[loop] Publishing a sample observation...");

  // Publish the observation to Sentilo Platform
  statusCode = sentiloClient.publishObservation(providerId, sensorId,	observation, apiKey, response);

  // Read response status and show an error if it is necessary
  if (statusCode != 0 && statusCode != 200) {
    Serial.print("[loop] [ERROR] Status code from server after publish the observations: ");
    Serial.println(statusCode);
    Serial.print("[loop] [ERROR] Response body from server after publish the observations: ");
    Serial.println(response);
  }

  Serial.println("[loop] Sample observation published!");
  Serial.println("[loop] Program ended");

  // The exmaple has ended,
  // so let execute an infinite loop
  while (true) { delay(1000);}
}


String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}



// Setup the Sentilo Client object
// This process also configures the network connection
void setupSetiloClient() {
  // Connect via DHCP
  Serial.print("[setup] Connecting to network via DHCP ");
  sentiloClient.dhcp();
  for (int i = 0; i < networkConnectionTimeout; i++) {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" done!");
  Serial.println("[setup] Connection is now established!");
}
