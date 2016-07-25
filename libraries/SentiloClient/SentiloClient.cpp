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
#include "SentiloClient.h"

char* CONTENT_TYPE = "application/json";
char* IDENTITY_KEY_HEADER = "identity_key";
char* CATALOG_BASE_PATH = "/catalog";
char* DATA_BASE_PATH = "/data";

SentiloClient::SentiloClient(const char* _host){
  host = _host;
  port = 80;
  num_headers = 0;
  contentType = CONTENT_TYPE;
}

SentiloClient::SentiloClient(const char* _host, int _port){
  host = _host;
  port = _port;
  num_headers = 0;
  contentType = CONTENT_TYPE;
}

void SentiloClient::dhcp() {
	byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	if (begin(mac) == 0) {
		CLIENT_DEBUG_PRINT("Failed to configure Ethernet using DHCP");
	}
	delay(1000);
}

int SentiloClient::begin(byte mac[]){
	//return Ethernet.begin(mac);
	delay(1000);
}

int SentiloClient::getCatalog(const char* apiKey, String &response) {
	CLIENT_DEBUG_PRINT("[SentiloClient::getCatalog] Retrieving catalog data");

	response = "";

	// Identity key header
	char identHeaderBuf[((unsigned)strlen(apiKey))+14+1];
	sprintf(identHeaderBuf, "%s: %s", IDENTITY_KEY_HEADER, apiKey);
	setHeader(identHeaderBuf);

	return get(CATALOG_BASE_PATH, &response);
}

int SentiloClient::publishObservation(const char* providerId, const char* sensorId, const SentiloClient::Observation &observation, const char* apiKey, String &response) {
	CLIENT_DEBUG_PRINT("[SentiloClient::publishObservation] Publishing observations");

	response = "";

	// Identity key header
	char identHeaderBuf[((unsigned)strlen(apiKey))+14+1];
	sprintf(identHeaderBuf, "%s: %s", IDENTITY_KEY_HEADER, apiKey);
	setHeader(identHeaderBuf);

	// Create the path
	char path[150];
	sprintf(path, "%s/%s/%s", DATA_BASE_PATH, providerId, sensorId);

	String observationInputMessage = createObservationInputMessage(observation);

	unsigned int inputMessageLength = observationInputMessage.length()+1;
	char buf[inputMessageLength];
	observationInputMessage.toCharArray(buf,inputMessageLength,0);

	return put(path, buf, &response);
}

int SentiloClient::registerSensor(const SentiloClient::Sensor &sensor, char* providerId, const char* apiKey, String &response) {
	CLIENT_DEBUG_PRINT("[SentiloClient::registerSensor] Registering a sensor");

	response = "";

	// Identity key header
	char identHeaderBuf[((unsigned)strlen(apiKey))+14+1];
	sprintf(identHeaderBuf, "%s: %s", IDENTITY_KEY_HEADER, apiKey);
	setHeader(identHeaderBuf);

	// Create the path
	unsigned int pathLength = strlen(CATALOG_BASE_PATH) + strlen(providerId) + 2;
	char path[pathLength];
	sprintf(path, "%s/%s", CATALOG_BASE_PATH, providerId);

	String sensorInputMessage = createSensorInputMessage(sensor);

	unsigned int inputMessageLength = sensorInputMessage.length()+1;
	char buf[inputMessageLength];
	sensorInputMessage.toCharArray(buf,inputMessageLength,0);

	return post(path, buf, &response);
}

String SentiloClient::createObservationInputMessage(const SentiloClient::Observation &observation) {
	String message = "{\"observations\":[{";

	message += "\"value\":\""+String(observation.value)+"\"";

	if (observation.timestamp != NULL && strlen(observation.timestamp) > 0) {
		message += ",\"timestamp\":\""+String(observation.timestamp)+"\"";
	}

	message += "}]}";

	return message;
}

String SentiloClient::createSensorInputMessage(const SentiloClient::Sensor &sensor) {

	String message = "{\"sensors\":[{";
	message += "\"sensor\":\""+String(sensor.sensor)+"\"";

	if (sensor.description != NULL && strlen(sensor.description) > 0) {
		message += ",\"description\":\""+String(sensor.description)+"\"";
	}

	if (sensor.type != NULL && strlen(sensor.type) > 0) {
		message += ",\"type\":\""+String(sensor.type)+"\"";
	}

	if (sensor.dataType != NULL && strlen(sensor.dataType) > 0) {
		message += ",\"dataType\":\""+String(sensor.dataType)+"\"";
	}

	if (sensor.unit != NULL && strlen(sensor.unit) > 0) {
		message += ",\"unit\":\""+String(sensor.unit)+"\"";
	}

	if (sensor.component != NULL && strlen(sensor.component) > 0) {
		message += ",\"component\":\""+String(sensor.component)+"\"";
	}

	if (sensor.componentType != NULL && strlen(sensor.componentType) > 0) {
		message += ",\"componentType\":\""+String(sensor.componentType)+"\"";
	}

	if (sensor.componentDesc != NULL && strlen(sensor.componentDesc) > 0) {
		message += ",\"componentDesc\":\""+String(sensor.componentDesc)+"\"";
	}

	if (sensor.location != NULL && strlen(sensor.location) > 0) {
		message += ",\"location\":\""+String(sensor.location)+"\"";
	}

	if (sensor.timeZone != NULL && strlen(sensor.timeZone) > 0) {
		message += ",\"timeZone\":\""+String(sensor.timeZone)+"\"";
	}

	message += "}]}";

	return message;
}

int SentiloClient::get(const char* path, String* response){
  return request("GET", path, NULL, response);
}

int SentiloClient::post(const char* path, const char* body, String* response){
	return request("POST", path, body, response);
}

int SentiloClient::put(const char* path, const char* body, String* response){
  return request("PUT", path, body, response);
}

int SentiloClient::request(const char* method, const char* path,
                  const char* body, String* response){

  CLIENT_DEBUG_PRINT("HTTP: connect\n");

  if(client.connect(host, port)){
    CLIENT_DEBUG_PRINT("HTTP: connected\n");
    CLIENT_DEBUG_PRINT("REQUEST: \n");
    // Make a HTTP request line:
    write(method);
    write(" ");
    write(path);
    write(" HTTP/1.1\r\n");
    for(int i=0; i<num_headers; i++){
      write(headers[i]);
      write("\r\n");
    }
    write("Host: ");
    write(host);
    write("\r\n");
    write("Connection: close\r\n");

    if(body != NULL){
      char contentLength[30];
      sprintf(contentLength, "Content-Length: %d\r\n", strlen(body));
      write(contentLength);

	  write("Content-Type: ");
	  write(contentType);
	  write("\r\n");
    }

    write("\r\n");

    if(body != NULL){
      write(body);
      write("\r\n");
      write("\r\n");
    }

    //make sure you write all those bytes.
    delay(100);

    CLIENT_DEBUG_PRINT("HTTP: call readResponse\n");
    int statusCode = readResponse(response);
    CLIENT_DEBUG_PRINT("HTTP: return readResponse\n");

    //cleanup
    CLIENT_DEBUG_PRINT("HTTP: stop client\n");
    num_headers = 0;
    client.stop();
    delay(50);
    CLIENT_DEBUG_PRINT("HTTP: client stopped\n");

    return statusCode;
  }else{
    CLIENT_DEBUG_PRINT("HTTP Connection failed\n");
    return 0;
  }
}

int SentiloClient::readResponse(String* response) {

  // an http request ends with a blank line
  boolean currentLineIsBlank = true;
  boolean httpBody = false;
  boolean inStatus = false;

  char statusCode[4];
  int i = 0;
  int code = 0;

  if(response == NULL){
    CLIENT_DEBUG_PRINT("HTTP: NULL RESPONSE POINTER: \n");
  }else{
    CLIENT_DEBUG_PRINT("HTTP: NON-NULL RESPONSE POINTER: \n");
  }

  CLIENT_DEBUG_PRINT("HTTP: RESPONSE: \n");
  while (client.connected()) {
    CLIENT_DEBUG_PRINT(".");

    if (client.available()) {
      CLIENT_DEBUG_PRINT(",");

      char c = client.read();
      CLIENT_DEBUG_PRINT(c);

      if(c == ' ' && !inStatus){
        inStatus = true;
      }

      if(inStatus && i < 3 && c != ' '){
        statusCode[i] = c;
        i++;
      }
      if(i == 3){
        statusCode[i] = '\0';
        code = atoi(statusCode);
      }

      if(httpBody){
        //only write response if its not null
        if(response != NULL) response->concat(c);
      }
      else
      {
          if (c == '\n' && currentLineIsBlank) {
            httpBody = true;
          }

          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          }
          else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
      }
    }
  }

  CLIENT_DEBUG_PRINT("HTTP: return readResponse3\n");
  return code;
}

void SentiloClient::write(const char* string){
  CLIENT_DEBUG_PRINT(string);
  client.print(string);
}

void SentiloClient::setHeader(const char* header){
  headers[num_headers] = header;
  num_headers++;
}

void SentiloClient::setContentType(const char* contentTypeValue){
  contentType = contentTypeValue;
}
