/*
   This is adapted from the Arduino example:
         Ethernet > BarometricPressureWebServer
   Intent is to read from a Hydreon Rain Gauge Model RG-11
*/

#include <Ethernet.h>


// Assign a MAC address for the Ethernet controller. Since the Arduino
// doesn't have its own MAC address assigned at the factory, you must
// pick one. Read more:
//
//     https://serverfault.com/a/40720
//     https://en.wikipedia.org/wiki/MAC_address#Universal_vs._local
//
// Quoting:
//
//     Universally administered and locally administered addresses are
//     distinguished by setting the second least significant bit of the
//     most significant byte of the address. If the bit is 0, the address
//     is universally administered. If it is 1, the address is locally
//     administered. In the example address 02-00-00-00-00-01 the most
//     significant byte is 02h. The binary is 00000010 and the second
//     least significant bit is 1. Therefore, it is a locally
//     administered address. The bit is 0 in all OUIs.
//
// So, ensure the low-nibble of the first byte is one of these 8
// hexidecimal values:
//
//    2  == 0010b
//    3  == 0011b
//    6  == 0110b
//    7  == 0111b
//    a  == 1010b
//    b  == 1011b
//    e  == 1110b
//    f  == 1111b
//
// There are websites that will generate these for you. Search for:
//
//    locally administered mac address generator
//
// TODO(james): Consider storing the address in the Arduino's EEPROM,
// generating a random address if it isn't there when first started.
// That would allow for this sketch to be copied many times, and used
// without editing.

// Fill in your address here:
byte mac[] = {
  0x52, 0xC4, 0x58, 0xC7, 0x2E, 0x81
};

// assign an IP address for the controller:
IPAddress ip(192, 168, 86, 251);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

float temperature = 0.0;
long pressure = 0;
long lastReadingTime = 0;

void setup() {
  // As described on the freetronics website, there is a delay between reset
  // of the microcontroller and the ethernet chip. Do some other stuff first
  // so that we don't try initializing the Ethernet chip too soon.
  //
  //       https://www.freetronics.com.au/pages/usb-power-and-reset

  long startTime = millis();

  // Open serial communications and wait for port to open, but not forever.
  Serial.begin(9600);
  // Wait for serial port to connect. Needed for native USB port only.
  while (!Serial && (millis() - startTime) < 120UL * 1000) {
    blink(200);
  }

  // Allow some time (250 ms) after powerup and sketch start, for the
  // Wiznet W5100 Reset IC to release and come out of reset.
  while ((millis() - startTime) < 250) {
    blink(100);
  }

  Serial.println("Setting up Ethernet pins");

  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  Serial.println("Setting up Ethernet library");

  // start the Ethernet connection
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    while (true) {
      Serial.println("Ethernet shield was not found.");
      Serial.println("Sorry, can't run without hardware. :(");
      Serial.println();
      delay(1000); // do nothing, no point running without Ethernet hardware
    }
  }
  Serial.println("Found ethernet hardware");

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  } else {
    Serial.println("Ethernet link is connected");
  }

  // start listening for clients
  server.begin();

  // give the sensor and Ethernet shield time to set up:
  delay(1000);

  // Silly pseudo readings.
  unsigned long t1 = millis();
  unsigned long t2 = micros();
  unsigned long a0 = analogRead(0);
  unsigned long a1 = analogRead(1);
  unsigned long a2 = analogRead(2);
  unsigned long a3 = analogRead(3);
  unsigned long a4 = analogRead(4);
  unsigned long a5 = analogRead(5);
  unsigned long seed = t1 + t2 + a0 + a1 + a2 + a3 + a4 + a5;
  Serial.print("Seed inputs: ");
  Serial.print(t1);
  Serial.print(", ");
  Serial.print(t2);
  Serial.print(", ");
  Serial.print(a0);
  Serial.print(", ");
  Serial.print(a1);
  Serial.print(", ");
  Serial.print(a2);
  Serial.print(", ");
  Serial.print(a3);
  Serial.print(", ");
  Serial.print(a4);
  Serial.print(", ");
  Serial.print(a5);
  Serial.println();
  Serial.print("Seed: ");
  Serial.println(seed);
  randomSeed(seed);


  temperature = random(-400, 1200) / 10.0;
  pressure = random(800, 1100);

}

unsigned long nextBlink = 0;
bool ledIsOn = false;

void loop() {
  // check for a reading no more than once a second.
  if (millis() - lastReadingTime > 1000) {
    Serial.println("We should read from our FAKE sensor!");

    temperature += random(-100, 100) / 100.0;
    pressure += random(-100, 100) / 100.0;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" degrees F");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.println();

    lastReadingTime = millis();
  }

  blink(1000);

  // Show a heartbeat.
  if (false && nextBlink <= millis()) {
    nextBlink += 1000;
    digitalWrite(LED_BUILTIN, ledIsOn ? LOW : HIGH);
    ledIsOn = !ledIsOn;
  }

  // listen for incoming Ethernet connections:
  listenForEthernetClients();
}

void blink(unsigned long interval) {
  static unsigned long lastBlink = 0;
  unsigned long nextBlink = lastBlink + interval;
  unsigned long now = millis();

//  Serial.print("lastBlink=");
//  Serial.print(lastBlink);
//  Serial.print(", nextBlink=");
//  Serial.print(nextBlink);
//  Serial.print(", now=");
//  Serial.print(now);

  if (nextBlink < lastBlink) {
    // Wrapped around.
    if (now >= lastBlink) {
      // Serial.println("   millis hasn't wrapped yet, so not time.");
      return;
    } else if (now < nextBlink) {
//      Serial.println("   Wrapped, but not time yet.");
      return;
    }
  } else if (now < lastBlink) {
    // Clock has wrapped around, so past due for blinking!
  } else if (now < nextBlink) {
    // Not time yet.
//    Serial.println("   Not time yet.");
    return;
  }

  digitalWrite(LED_BUILTIN, ledIsOn ? LOW : HIGH);
  ledIsOn = !ledIsOn;
  lastBlink = now;
//  Serial.println("    BLINK!");
}


void listenForEthernetClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Got a client!!");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // print the current readings, in HTML format:
          client.print("Temperature: ");
          client.print(temperature);
          client.print(" degrees C");
          client.println("<br />");
          client.print("Pressure: " + String(pressure));
          client.print(" Pa");
          client.println("<br />");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
