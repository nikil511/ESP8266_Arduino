/*
 * Sentilo
 *
 * Copyright (C) 2013 Institut Municipal d’Informàtica, Ajuntament de Barcelona.
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
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

#ifdef HTTP_DEBUG
#define CLIENT_DEBUG_PRINT(string) (Serial.print(string))
#endif

#ifndef HTTP_DEBUG
#define CLIENT_DEBUG_PRINT(string)
#endif

class SentiloClient {

public:

	struct Sensor {
		char* sensor = NULL;
		char* description = NULL;
		char* type = NULL;
		char* dataType = NULL;
		char* unit = NULL;
		char* component = NULL;
		char* componentType = NULL;
		char* componentDesc = NULL;
		char* location = NULL;
		char* timeZone = NULL;
	};

	struct Observation {
		char* value = NULL;
		char* timestamp = NULL;
	};

	SentiloClient(const char* host);
	SentiloClient(const char* host, int port);

	void dhcp();
	int begin(byte*);

	int getCatalog(const char* apiKey, String &response);
	int publishObservation(const char* providerId, const char* sensorId, const SentiloClient::Observation &observation, const char* apiKey, String &response);
	int registerSensor(const SentiloClient::Sensor &sensor, char* providerId, const char* apiKey, String &response);

private:
	WiFiClient  client;
	const char* host;
	int port;
	int num_headers;
	const char* headers[10];
	const char* contentType;

	int request(const char* method, const char* path, const char* body, String* response);
	void setHeader(const char*);
	void setContentType(const char*);

	int readResponse(String*);
	void write(const char*);

	int get(const char*, String*);
	int post(const char* path, const char* body, String*);
	int put(const char* path, const char* body, String*);

	String createObservationInputMessage(const SentiloClient::Observation &observation);
	String createSensorInputMessage(const SentiloClient::Sensor &sensor);
};
