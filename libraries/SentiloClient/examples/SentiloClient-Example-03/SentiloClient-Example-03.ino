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
int LDR = 0;                          // LDR input is A0
int LM35 = 5;                         // LM35 input is A5
const int ldrSetupTimeout = 10; // Time that LDR needs to be configures (dummy time)
const int lm35SetupTimeout = 10; // Time that LM35 needs to be configures (dummy time)

/*******************************************/
/***** SENTILO *****************************/
/*******************************************/
char* apiKey = "YOUR_API_KEY";
char* ip = "YOUR_IP_ADDRESS";
int port = YOUR_PORT;
char* componentId = "sample-component";
char* providerId = "samples-provider";
char* sensorId = "sample-sensor-arduino-03";
char* sensorLocation = "41.3850639 2.1734035";

// The Sentilo Client object
SentiloClient sentiloClient = SentiloClient(ip, port);

/*******************************************/
/***** NETWORK *****************************/
/*******************************************/
const int networkConnectionTimeout = 30;

/*******************************************/
/***** BGLOBAL VARS ************************/
/*******************************************/
const int generalCalibrationTimeout = 1000; 	// Wait after system setup is complete
const int loopTimeout = 60000; 					// Loop timeout, time between observations publications (in ms)
String response = "";            				// Rest call response (normaly as JSON message)
int statusCode = -1;                   			// Rest call return code (the HTTP code)

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

	// Setup the Sentilo sensor 
	// and create it if doesn't exists
	setupSentiloSensor();

	// Waiting for the next release of the observation
	delay(generalCalibrationTimeout);
}

void loop() {
	if (existsSensor) {
		// If the sensor exists, 
		// we can start publishing observations
		
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
	} else {
		// If the sensor does not exist and it could 
		// not be created in the catalog, we must stop running
		Serial.println("[loop] [ERROR] Oops! The sensor doesn't exists, so I can't publish data to it...");
		Serial.println("[loop] [ERROR] I'm sorry with you, but now I'm going to halt...");
		Serial.println("[loop] [ERROR] Bye!");
		while (true) { }
	}
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

// Setup the Sentilo Sentor (this Arduino)
// If the sensor doesn't exists in the catalog, create it
void setupSentiloSensor() {
	Serial.println("[setup] Retrieving catalog info from Sentilo and search for the sensor...");

	// Get catalog data for the provider with the supplied api key
	statusCode = sentiloClient.getCatalog(apiKey, response);

	// If the server status response is not ok, show the error
	if (statusCode != 200) {
		Serial.print("[setup] [ERROR] Status code from server getting catalog: ");
		Serial.println(statusCode);
		Serial.print("[setup] [ERROR] Response body from server getting catalog: ");
		Serial.println(response);
	} else {
		// If we get a correct response, we must search the sensor
		if (find_text(sensorId, response) >= 0) {
			// The sensor is in the catalog
			Serial.println("[setup] The sensor is in the catalog");
			existsSensor = true;
		} else {
			// The sensor isn't in the catalog, so we must create it
			Serial.println("[setup] The sensor isn't in the catalog, so let register it now...");

			// Create the basic Sentilo Sensor Object
			SentiloClient::Sensor sensor;
			sensor.sensor = sensorId;
			sensor.type = "status";
			sensor.dataType = "TEXT";
			sensor.component = componentId;
			sensor.componentType = "generic";
			sensor.location = sensorLocation;

			// Call the SentiloClient Register Sensor function
			statusCode = sentiloClient.registerSensor(sensor, providerId, apiKey, response);

			// Read the server status response
			if (statusCode == 200) {
				// If ok, the sensor has been yet created
				existsSensor = true;
			} else {
				// If nok, then we can't continue with the program
				existsSensor = false;
				Serial.print("[setup] [ERROR] Status code from server getting catalog: ");
				Serial.println(statusCode);
				Serial.print("[setup] [ERROR] Response body from server getting catalog: ");
				Serial.println(response);
			}
		}
	}
}

// Auxiliary function for search text in a String
int find_text(String needle, String haystack) {
	int foundpos = -1;
	for (int i = 0; (i < haystack.length() - needle.length()); i++) {
		if (haystack.substring(i, needle.length() + i) == needle) {
			foundpos = i;
		}
	}
	return foundpos;
}
