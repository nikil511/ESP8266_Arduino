/*
 * Sentilo
 * 
 * Copyright (C) 2013 Institut Municipal d�Inform�tica, Ajuntament de Barcelona.
 * 
 * This program is licensed and may be used, modified and redistributed under the terms of the
 * European Public License (EUPL), either version 1.1 or (at your option) any later version as soon
 * as they are approved by the European Commission.
 * 
 * Alternatively, you may redistribute and/or modify this program under the terms of the GNU Lesser
 * General Public License as published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied.
 * 
 * See the licenses for the specific language governing permissions, limitations and more details.
 * 
 * You should have received a copy of the EUPL1.1 and the LGPLv3 licenses along with this program;
 * if not, you may find them at:
 * 
 * https://joinup.ec.europa.eu/software/page/eupl/licence-eupl http://www.gnu.org/licenses/ and
 * https://www.gnu.org/licenses/lgpl.txt
 */
#include <Ethernet.h>
#include <SPI.h>

#include "SentiloClient.h"

/*******************************************/
/***** SENSORS *****************************/
/*******************************************/
int LDR = 0;                          	// LDR input is A0
int LM35 = 5;                         	// LM35 input is A5
const int ldrSetupTimeout = 10; 		// Time that LDR needs to be configures (dummy time)
const int lm35SetupTimeout = 10; 		// Time that LM35 needs to be configures (dummy time)

/*******************************************/
/***** SENTILO *****************************/
/*******************************************/
char* apiKey = "YOUR_API_KEY";
char* ip = "YOUR_IP_ADDRESS";
int port = YOUR_PORT;
char* componentId = "sample-component";
char* providerId = "samples-provider";
char* sensorId = "sample-sensor-arduino-02";

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
const int loopTimeout = 60000; 					// Loop timeout, time between observations publishment (in ms)
String response = "";            				// Rest call response (normaly as JSON message)
int statusCode = -1;                    		// Rest call return code (the HTTP code)

boolean existsSensor = false;

void setup() {
	// Begin serial for debug purposes
	Serial.begin(9600);

	// Setup the LDR sensor
	setupLDR();

	// Setup the LM35 sensor
	setupLM35();

	// Setup the Sentilo Client 
	// and network connection
	setupSetiloClient();

	// Wait time for a general calibration
	delay(generalCalibrationTimeout);
}

void loop() {
	// Get the LDR value  
	int ldrValue = getLdrValue();

	// Get the LM35 value
	float lm35Value = getLM35Value();

	// Create the observation input message
	// like this: {"ldr":"234","lm35":"24.5"}
	String obsInputMsg = 
			"{\\\"ldr\\\":\\\"" + String(ldrValue) + 
			"\\\",\\\"lm35\\\":\\\"" + String(lm35Value) + 
			"\\\"}";
	int bufLength = obsInputMsg.length() + 1;
	char obsMsgBuffer[bufLength];
	obsInputMsg.toCharArray(obsMsgBuffer, bufLength);

	// Create the Observation object
	SentiloClient::Observation observation;
	observation.value = obsMsgBuffer;

	// Debug on Serial the observations value
	// Note that the message includes slashes (\) because we must scape special characters as "
	Serial.print("[loop] Publishing actual sensors values as observations: ");
	Serial.println(obsMsgBuffer);
	
	// Publish the observation to Sentilo Platform
	statusCode = sentiloClient.publishObservation(providerId, sensorId, observation, apiKey, response);

	// Read response status and show an error if it is necessary
	if (statusCode != 0 && statusCode != 200) {
		Serial.print("[loop] [ERROR] Status code from server after publish the observations: ");
		Serial.println(statusCode);
		Serial.print("[loop] [ERROR] Response body from server after publish the observations: ");
		Serial.println(response);
	} else {
		Serial.println("[loop] Sensors observations published!");
	}
	
	// Waiting for the next release of the observation
	delay(loopTimeout);
}

// Emulate a possible LDR initialization process, if it is necessary
void setupLDR() {
	Serial.print("[setup] Setting up the LDR brightness sensor ");
	for (int i = 0; i < ldrSetupTimeout; i++) {
		Serial.print(".");
		delay(100);
	}
	Serial.println(" done!");
	delay(50);
}

// Get the brightness value from th LDR
int getLdrValue() {
	return analogRead(LDR);
}

// Emulate a possible LM35 initialization process, if it is necessary
void setupLM35() {
	Serial.print("[setup] Setting up the LM35 temperature sensor ");
	for (int i = 0; i < lm35SetupTimeout; i++) {
		Serial.print(".");
		delay(100);
	}
	Serial.println(" done!");
	delay(50);
}

// Get the LM 35 temperature value in Celsius degrees
float getLM35Value() {
	int val = analogRead(LM35);
	float mv = (val / 1024.0) * 5000;
	float cel = mv / 10;
	//float farh = (cel * 9) / 5 + 32;
	return cel;
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
