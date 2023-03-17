//ARDUINO CODE USING ETHERNET SHEILD TO PERFORM PERIODIC HTTP PINGS TO THE UUID OF YOUR CHOICE

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Ethernet.h> 
#include <avr/wdt.h>
#include <Dns.h>

//USER DEFINED VARIABLES
IPAddress subnet(255,255,255,0);
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05 };       //MAC address assigned to the Arduino Ethernet Shield
long interval_heartbeat = 900000;                         // READING INTERVAL --> how often is healthchecks.io contacted for a heartbeat update? value = 15 minutes 

//SYSTEM VARIABLES
IPAddress healthcheckio_IP;
EthernetClient client;
DNSClient dnClient;
long previousMillis_heartbeat = 0;
unsigned long currentMillis = 0;

//SETUP INITILIZATION 
void setup() { 
	Serial.begin(115200);
	Serial.println(F("Initialize Ethernet with DHCP:"));

	if (Ethernet.begin(mac) == 0) {
		Serial.println(F("Failed to configure Ethernet using DHCP"));
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
		} else if (Ethernet.linkStatus() == LinkOFF) {
			Serial.println(F("Ethernet cable is not connected."));
		}
		// no point in carrying on, so do nothing forevermore:
		while (true) {
			delay(10000);
			Serial.println(F("Reset or Reboot the Arduino to try DHCP again"));
		}
	}

	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());
  
	dnClient.begin(Ethernet.dnsServerIP());
	if(dnClient.getHostByName("hc-ping.com",healthcheckio_IP) == 1) {
		Serial.print(F("hc-ping.com = "));
		Serial.println(healthcheckio_IP);
		Serial.println("");
		Serial.println("");
	}else{ 
		Serial.print(F("dns lookup failed"));
	}
 
	wdt_enable(WDTO_4S);
}

void loop(){
	//maintain DHCP IP lease
	switch (Ethernet.maintain()) {
    case 1:
		//renewed fail
		Serial.println(F("Error: renewed fail"));
		break;
    case 2:
		//renewed success
		Serial.println(F("Renewed success"));
		Serial.print(F("My IP address: "));
		Serial.println(Ethernet.localIP());
		break;
    case 3:
		//rebind fail
		Serial.println(F("Error: rebind fail"));
		break;
    case 4:
		//rebind success
		Serial.println(F("Rebind success"));
		Serial.print(F("My IP address: "));
		Serial.println(Ethernet.localIP());
		break;
    default:
		//nothing happened
		break;
	}
    wdt_reset();
	
	currentMillis = millis();
     
	if(currentMillis - previousMillis_heartbeat > interval_heartbeat) { // PERFORM ONLY ONCE PER INTERVAL
		previousMillis_heartbeat = currentMillis; 
		if(dnClient.getHostByName("hc-ping.com",healthcheckio_IP) == 1) {
			Serial.print(F("hc-ping.com = "));
			Serial.println(healthcheckio_IP);
		}else{
			Serial.print(F("dns lookup failed"));
		}

		if (client.connect(healthcheckio_IP,80)) {
			Serial.println(F("Heartbeat Client Connected"));
			Serial.println("");
			client.println(F("GET /xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx HTTP/1.1"));  //replace with UUID of choice
			client.println(F("Host: hc-ping.com"));
			client.println(F("Connection: close"));
			client.println();
			client.stop();
		} else{
			Serial.print(F("Heartbeat could not connect to server"));
		}
	}
}
//END OF FILE
