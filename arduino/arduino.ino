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
int USE_WIFI = 1;

// HTTP request stuff
#define BASE_URL         "173.17.168.19"
#define URL_ADD          "/lab1/temperature/add?temp="

char buf[8];
char resBuffer[12] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int resBufferSize = 0;

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
  // set up the one wire temperature reading
  sensors.begin();
  sensors.requestTemperatures();
  
  float val = sensors.getTempCByIndex(0);
  Serial.println();
  Serial.print("Current Temperature: ");
  Serial.println(val);
  
  if (val != -127.0f) {
    if (connected == 1) {
      //Serial.println("Requesting Connection");
      makeRequest(val);
      delay(700);
    }
  } else {
    Serial.println("Not connected");
    if (connected == 1) {
      makeRequest(404);
      delay(700);
    }
  }
}

/*********************************************************
/ Make a new network request by opening up the pipe,
/ sending the information, then closing the pipe.
/********************************************************/
void makeRequest(float currentTemp) {
  // Actually make request to webpage
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 8083);
  //Serial.println("Connecting to TCP");
  if (!requestEndpoint(currentTemp, www)) {
    //Serial.println("Could not connect to endpoint");
    return;
  }
  
  //Serial.println("Connected to endpoint");
  processResponse(www);
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
    Serial.print(F("GET "));
    www.fastrprint(F("GET "));
    Serial.print(URL_ADD);
    www.fastrprint(URL_ADD);
    dtostrf(value, 6, 3, buf);
    Serial.print(buf);
    www.fastrprint(buf);
    Serial.print(F(" HTTP/1.1\r\n"));
    www.fastrprint(F(" HTTP/1.1\r\n"));
    Serial.print(F("Host: "));
    www.fastrprint(F("Host: "));
    Serial.print(BASE_URL);
    www.fastrprint(BASE_URL);
    Serial.print(F("\r\n"));
    www.fastrprint(F("\r\n"));
    Serial.print(F("User-Agent: Arduino\r\n"));
    www.fastrprint(F("User-Agent: Arduino\r\n"));
    Serial.print(F("Content-Type: application/json\r\n"));
    www.fastrprint(F("Content-Type: application/json\r\n"));
    Serial.print(F("\r\n"));
    www.fastrprint(F("\r\n"));
    Serial.println();
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

  // Read data until either the connection is closed, or the idle timeout is reached.
  Serial.println(F("-------------------------------------"));
  
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      shiftBufferLeft(c);
      lastRead = millis();
    }
    
    // TODO enable/disable the LED array based on this value
    Serial.println("Result: ");
    Serial.println(resBuffer[4]);
    Serial.println();
    break;
  }
  
  www.close();
  Serial.println(F("-------------------------------------"));
}

/*********************************************************
/ Shift the circular buffer for response reading.
/********************************************************/
void shiftBufferLeft(char c) {
  if (resBufferSize == 12) {
    resBuffer[0] = resBuffer[1];
    resBuffer[1] = resBuffer[2];
    resBuffer[2] = resBuffer[3];
    resBuffer[3] = resBuffer[4];
    resBuffer[4] = resBuffer[5];
    resBuffer[5] = resBuffer[6];
    resBuffer[6] = resBuffer[7];
    resBuffer[7] = resBuffer[8];
    resBuffer[8] = resBuffer[9];
    resBuffer[9] = resBuffer[10];
    resBuffer[10] = resBuffer[11];
    resBuffer[11] = c;
  } else {
    resBuffer[resBufferSize] = c;
    resBufferSize++;
  }
}
