#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <string.h>
#include <Adafruit_CC3000.h>

// definitions for one wire temp sensor
#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// definitions for wifi
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER);
                         
// WiFi security declarations
#define WLAN_SSID       "APT23-2"          // This needs to be your WiFi access point name, 32 character max
#define WLAN_PASS       "waterpolo"        // WiFi access point password
#define WLAN_SECURITY   WLAN_SEC_WPA2      // WiFi security type
#define IDLE_TIMEOUT_MS 3000
int connected;
uint32_t ip;
int USE_WIFI = 0;

// HTTP request stuff
#define BASE_URL         "173.17.168.19"
#define URL_ADD          "/lab1/temperature/add?temp="

/*********************************************************
/ Set up all communication with the arduino here
/     1) Serial Communication
/     2) Temperature Sensor
/     3) WiFi
/     4) Push Button
/     5) LEDs
/********************************************************/
void setup() {
  // set up serial communication
  Serial.begin(9600);
  
  // set up the one wire temperature reading
  sensors.begin();
  
  // for debugging purposes... the school wifi cannot be connected to because
  // it requires both a username and password. You need an actual os to be able
  // to handle these requests, arduino isn't powerful enough to do so. So,
  // we'll need to use a hotspot to get around this. For testing, just don't
  // connect at all so we can still work on the hardware without the hotspot
  if (USE_WIFI == 0) {
    Serial.println("Skipping connecting to WiFi");
    return;
  }  
  
  // set up the wifi module
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }
  
  // Connect to WiFi access point
  Serial.print(F("\nAttempting to connect to "));
  Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    connected = 0;
    return;
  }

  connected = 1;
  Serial.println(F("Connected!"));

  // Wait for DHCP to complete
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100);
  }

  // Display the IP address DNS, Gateway, etc.
  while (!displayConnectionDetails()) {
    delay(1000);
  }

  ip = 0;
  // Try looking up the website's IP address
  Serial.print(BASE_URL);
  Serial.print(F(" -> "));
  while (ip == 0) {
    if (!cc3000.getHostByName(BASE_URL, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }

    delay(500);
  }

  cc3000.printIPdotsRev(ip);
}

/*********************************************************
/ Continue looping through this program until the power
/ is turned off. This will read the current temperature
/ value (if available) and print it to the serial output.
/
/ After printing to serial, if connected to WiFi, we
/ should create a request to post the information to our
/ REST endpoint so that it can be viewed on the web.
/********************************************************/
void loop() {
  sensors.requestTemperatures();
  float val = sensors.getTempCByIndex(0);
  
  if (val != -127.0) {
    Serial.print("Current Temperature: ");
    Serial.println(val);
  
    if (connected == 1) {
      makeRequest(val);
    }
  } else {
    Serial.println("Not connected to sensor."); 
    delay(1000);
  }
}

/*********************************************************
/ Make a new network request by opening up the pipe,
/ sending the information, then closing the pipe.
/********************************************************/
void makeRequest(float currentTemp) {
  // Actually make request to webpage
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 8083);
  if (!requestEndpoint(currentTemp, www)) {
    return;
  }
  
  processResponse(www);
  www.close();
}

/*********************************************************
/ Display the wifi connection details once we successfully
/ connect.
/********************************************************/
bool displayConnectionDetails(void) {
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv)) {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  } else {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/*********************************************************
/ Request a POST to the endpoint for adding temperature
/ values.
/********************************************************/
bool requestEndpoint(float value, Adafruit_CC3000_Client& www) {
  if (www.connected()) {
    www.fastrprint(F("POST "));
    www.fastrprint(URL_ADD);
    char* charVal;
    dtostrf(value, 4, 3, charVal);
    www.fastrprint(charVal);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: "));
    www.fastrprint(BASE_URL);
    www.fastrprint(F("\r\n"));
    www.fastrprint(F("User-Agent: Arduino\r\n"));
    www.fastrprint(F("Content-Type: application/json\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
    return true;
  } else {
    Serial.println(F("Connection failed"));
    return false;
  }
}

/*********************************************************
/ Process the response from the request.
/********************************************************/
void processResponse(Adafruit_CC3000_Client& www) {
  Serial.println();
  Serial.println(F("-------------------------------------"));

  // Read data until either the connection is closed, or the idle timeout is reached.
  unsigned long lastRead = millis();
    
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    bool found = false;
    bool needed = true;
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      
      // TODO we need to check here what the response is from the endpoint
      //      a 1 means that we should enable the LEDs, a 0 means we should
      //      disable them.
    }
  }
  
  Serial.println();
  Serial.println(F("-------------------------------------"));
}
