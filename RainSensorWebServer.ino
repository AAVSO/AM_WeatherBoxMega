// HTTP server offering readings from a Hydreon Rain Gauge Model RG-11 (i.e. the
// relay state) and from a Melexis MLX90614 IR Sensor, used as a sky temperature
// sensor; clouds are warmer than clear sky due to the water in them, so this
// allows us to estimate when there are clouds.
//
// Author: James Synge
// Date: March, 2019
// Adapted from the Arduino example "Ethernet > BarometricPressureWebServer".
// See here for an expanded and heavily documented example:
//    https://github.com/jamessynge/arduino_experiments/tree/master/sensor_ether_server

#include <Arduino.h>

// This is the Arduino "standard" Ethernet 2.0.0 library (or later).
#include <Ethernet.h>

// Multicast DNS support, aka Apple's Bonjour protocol.
// I've forked this library just so that I'm sure I can find it:
// https://github.com/jamessynge/EthernetBonjour
#include <EthernetBonjour.h>

#include "SparkFunMLX90614.h"

// TODO Look into moving my helpers into a library directory, and symlinking the
// library directory into the sketch directory. That will make sharing the
// files easier.
#include "simple_http_server.h"

// Pin hooked up to the RG-11 rain sensor's relay.
constexpr int kRelayInputPin = 7;

// Class instance for reading from the attached MLX90614 IR Sensor.
IRTherm irTherm;

// Name we'll advertise using mDNS (Apple's Bonjour protocol).
const char kMulticastDnsName[] = "rainsensor";

// Tell the Ethernet library to talk to the Ethernet chip using the
// appropriate chip select pin, and listen on port 80 for HTTP connections.
SimpleHttpServer server(kEthernetShieldCS, 80);

void setup() {
  Serial.begin(9600);

  // We've got our own external pullup resistor, so not setting this
  // to INPUT_PULLUP.
  pinMode(kRelayInputPin, INPUT);

  // Start talking to the IR sensor.
  irTherm.begin();
  irTherm.setUnit(TEMP_F);
  if (irTherm.read() != 1) {
    Serial.println("MLX90614 not found");
  }

  // As described on the freetronics website, there is a delay between the reset
  // of the EtherTen board and the time when the Ethernet chip is allowed to
  // operate. So we do some other stuff first (above), then delay a bit longer
  // just to be sure that we don't attempt to initialize the Ethernet chip too
  // soon. Waiting a few 100ms will not be a big deal in the life of the unit.
  //
  //       https://www.freetronics.com.au/pages/usb-power-and-reset
  delay(200);

  // Initialize networking. Provide an "Organizationally Unique Identifier"
  // that will be the first 3 bytes of the MAC addresses generated; this means
  // that all boards running this sketch will share the first 3 bytes of their
  // MAC addresses.
  uint8_t oui_prefix[3] = {0x52, 0xC4, 0x55};
  if (!server.setup(oui_prefix)) {
    announceFailure("Unable to initialize networking!");
  }

  if (!EthernetBonjour.begin(kMulticastDnsName)) {
    Serial.println("No mDNS! continuing");
  }
}

void clientHandler(EthernetClient* client) {
  Serial.println("Got a client!!");

  // Skip the request header, i.e. we give the same result regardless of the
  // request, so really not much of a webserver!
  server.skipHttpRequestHeader(client);

  // Send a basic HTTP response header:
  client->println("HTTP/1.1 200 OK");
  client->println("Content-Type: application/json");
  client->println();

  client->print("{\"relay\":");
  client->print((digitalRead(kRelayInputPin) == LOW) ? 0 : 1);

  // On average it takes just over one millisecond to read from the IR sensor,
  // so there is no need to do so periodically, (i.e. between client requests,
  // with caching of the results). So we just try to read and return the
  // results if we get them.
  if (irTherm.read() == 1) {
    client->print(", \"object\":");
    client->print(irTherm.object());
    client->print(", \"ambient\":");
    client->print(irTherm.ambient());
  }

  client->println("}");
  client->println();
}

void loop() {
  // If we've received an mDNS query for our name, respond. This must be called
  // often in order for the mDNS feature to work, ideally once per loop.
  EthernetBonjour.run();

  // If there is a client, pass it to clientHandler;
  // also maintain our DHCP lease.
  server.loop(clientHandler);
}

void announceFailure(const char* message) {
  while (true) {
    Serial.println(message);
    delay(1000);
  }
}
