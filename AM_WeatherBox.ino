// notes at the bottom


#include <Arduino.h>
#include "defines.h" 

#define ALPACA_PORT   80
#define ALPACA_DISCOVERY_PORT  32227

EthernetWebServer server(ALPACA_PORT);
#include <EthernetUdp3.h> //<WiFiUdp.h>
//todo  UDP Port can be edited in setup page
int udpPort = ALPACA_DISCOVERY_PORT;
EthernetUDP Udp;

#define DRIVER_VERSION  21 // must be int  "0.2.1" // this code

#include "alpaca.h"

// Multicast DNS support, aka Apple's Bonjour protocol.
// I've forked this library just so that I'm sure I can find it:
// https://github.com/jamessynge/EthernetBonjour
//#include <EthernetBonjour.h>

// Library for reading from the MLX90614 IR Sensor.
#include <SparkFunMLX90614.h>

// Provides the seed for the standard random number generator.
#include "jitter_random.h"

// Generates, stores and loads MAC and IP address for the sketch.
#include "addresses.h"

// Pin hooked up to the RG-11 rain sensor's relay.
constexpr int kRelayInputPin = 47;

// Class instance for reading from the attached MLX90614 IR Sensor.
IRTherm irTherm;

// Name we'll advertise using mDNS (Apple's Bonjour protocol).
//const char kMulticastDnsName[] = "rainsensor";



void setup() {
  Serial.begin(115200);
  Serial.println("start setup");

  pinMode(kRelayInputPin, INPUT_PULLUP); // internal input is adequate; lose the external pullup circuit

  // Start talking to the IR sensor.
  irTherm.begin();
  irTherm.setUnit(TEMP_F);
  if (irTherm.read() != 1) {
    Serial.println("MLX90614 not found");
  }

  // Initialize the random number generator, just in case we need it for
  // generating addresses when we call SimpleHttpServer::setup.
  // Serial.print("Calling JitterRandom at ");
  // Serial.println(millis());
  //auto seed = JitterRandom::random32();
  // Serial.print("JitterRandom returned at ");
  // Serial.println(millis());
  // Serial.print("seed=");
  // Serial.print(seed);
  // Serial.print(" (0x");
  // Serial.print(seed, HEX);
  // Serial.println(")");

//  randomSeed(seed);

  // As described on the freetronics website, there is a delay between the reset
  // of the EtherTen board and the time when the Ethernet chip is allowed to
  // operate. So we do some other stuff first (above), then delay a bit longer
  // just to be sure that we don't attempt to initialize the Ethernet chip too
  // soon. Waiting a few 100ms will not be a big deal in the life of the unit.
  // To learn more, see:
  //       https://www.freetronics.com.au/pages/usb-power-and-reset
  delay(200);
  
  pinMode(7, INPUT_PULLUP); // ethernet needs this

  // Initialize networking. Provide an "Organizationally Unique Identifier"
  // that will be the first 3 bytes of the MAC addresses generated; this means
  // that all boards running this sketch will share the first 3 bytes of their
  // MAC addresses, which may help with locating them... though EthernetBonjour
  // (mDNS) is our primary way of doing so.
//  OuiPrefix oui_prefix(0x52, 0xC4, 0x55);
//  if (!server.setup(&oui_prefix)) {
//    announceFailure("Unable to initialize networking!");
//  }

//  if (!EthernetBonjour.begin(kMulticastDnsName)) {
//    Serial.println("No mDNS! continuing");
//  }

 
 ET_LOGWARN3(F("Board :"), BOARD_NAME, F(", setCsPin:"), USE_THIS_SS_PIN);

    // Use  MAX_SOCK_NUM = 4 for 4K, 2 for 8K, 1 for 16K RX/TX buffer
    #ifndef ETHERNET3_MAX_SOCK_NUM
      #define ETHERNET3_MAX_SOCK_NUM      4
    #endif
  Serial.println("here01");
  
    Ethernet.setCsPin (USE_THIS_SS_PIN);
    Ethernet.init (ETHERNET3_MAX_SOCK_NUM);
  Serial.println("here02");

  
    // You have to add initialization for your Custom Ethernet here
    // This is just an example to setCSPin to USE_THIS_SS_PIN, and can be not correct and enough
    Ethernet.init(USE_THIS_SS_PIN);
    


pinMode(7, INPUT_PULLUP); // ethernet needs this
  Serial.println("here03");

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  uint16_t index = millis() % NUMBER_OF_MAC;
  // Use Static IP
  //Ethernet.begin(mac[index], ip);
  
  Ethernet.begin(mac[index]);
  Serial.println("here04");

  // Just info to know how to connect correctly
  Serial.println(F("========================="));
  Serial.println(F("Currently Used SPI pinout:"));
  Serial.print(F("MOSI:"));
  Serial.println(MOSI);
  Serial.print(F("MISO:"));
  Serial.println(MISO);
  Serial.print(F("SCK:"));
  Serial.println(SCK);
  Serial.print(F("SS:"));
  Serial.println(SS);
#if USE_ETHERNET3
  Serial.print(F("SPI_CS:"));
  Serial.println(SPI_CS);
#endif
  Serial.println(F("========================="));

  Serial.print(F("Using mac index = "));
  Serial.println(index);

  Serial.print(F("EthernetWebServer started @ IP address: "));
  Serial.println(Ethernet.localIP());

  // start the web server on port 80
  server.begin();
 
   alpaca_setup();

  server.on("/", oldhandler);

  //Starts the discovery responder server
  Udp.begin( udpPort);

  
} //setup

void oldhandler() {
  String msg= "{\"relay\":"+ String((digitalRead(kRelayInputPin) == LOW) ? 0 : 1);
  // On average it takes just over one millisecond to read from the IR sensor,
  // so there is no need to do so periodically, (i.e. between client requests,
  // with caching of the results). So we just try to read and return the
  // results if we get them.
  if (irTherm.read() == 1) {
    msg+= ", \"object\":"+ String(irTherm.object(), 2)+ ", \"ambient\":"+ String(irTherm.ambient(),2);
  }
  msg+= "}";
  server.send(200, "application/json", msg);
}

void loop() {
  server.handleClient();
  // If we've received an mDNS query for our name, respond. This must be called
  // often in order for the mDNS feature to work, ideally once per loop.
  //EthernetBonjour.run();
  int udpBytesIn = Udp.parsePacket();
  if( udpBytesIn > 0  ) 
     handleDiscovery( udpBytesIn );
}

void announceFailure(const char* message) {
  while (true) {
    Serial.println(message);
    delay(1000);
  }
}






//  board selected: Arduino Mega or Mega 2560
//
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
//
// Installation and testing procedure, using the Arduino IDE:
//
// 1) We need to install the SparkFunMLX90614 library into the IDE. Download the
//    zip file located at:
//      https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library/archive/master.zip
//
// 2) Install the SparkFunMLX90614 library into the IDE using this menu
//    selection:
//
//      Sketch  > Include Library > Add .ZIP Library...
//    Note that the list of libraries may be very long, so the menu of libraries
//
//    may end up scrolling such that you can't see the Add .ZIP Library menu
//    entry. It is near the top, so scroll the menu if you don't see it. Once
//    you've selected the menu entry, a file dialog will open; navigate to the
//    downloaded .zip file. Note that the URL makes it appear that you'll
//    download a file called master.zip, but I've noticed that the file is
//    actually called SparkFun_MLX90614_Arduino_Library-master.zip (i.e. it
//    is the name of the git repository, "SparkFun_MLX90614_Arduino_Library",
//    followed by "-master.zip").
//
// 3) Repeat the above steps for the Ethernet Bonjour library, located at:
//
//      https://github.com/TrippyLighting/EthernetBonjour/archive/master.zip
//
// 4) If you haven't already, download this sketch, including the helper files
//    in its directory (simple_http_server.h, etc.).
//
// 5) Open this sketch in the Arduino IDE (i.e. open RainSensorWebServer.ino).
//
// 6) Select the type of Arduino you will be programming with the menu:
//
//      Tools > Board
//
// 7) To ensure that we can compile the code (i.e. that all the correct code
//    has been installed where the Arduino IDE expects), click the Verify
//    button in the IDE (a big check mark), or select the menu entry:
//
//      Sketch > Verify/Compile
//
//    Once that is working, then we can go on to programming the Arduino.
//
// 8) Attach your Arduino to your computer for programming (i.e. connect via
//    USB). Note: I sometimes find it useful to first program the Arduino
//    example called Blink before moving on to something more complicated. That
//    helps to eliminate simple problems (bad cables, for example) before going
//    on to complex ones. For example, when developing this software I found
//    that one USB cable was good enough to power the Arduino, but not very good
//    for programming.
//
// 9) Connect an Ethernet cable to the Ethernet port of your Ethernet Shield
//    (or Ethernet capable Arduino clone, such as the freetronics EtherTen),
//    and connect the other end of the cable to your network, such as into a
//    port of your cable modem or router.
//
// 10) Select the serial port that your board appears at using this menu:
//
//       Tools > Port
//
//     For example, COM1 on Windows or /dev/ttyACM3 on Linux.
//
// 10) Upload the sketch to the Arduino by clicking the Upload button (a
//     big, right arrow) or by selecting the menu entry:
//
//       Sketch > Upload
//
// 11) Once the sketch is uploaded, select the menu:
//
//       Tools > Serial Monitor
//
//     You'll then be able to see the output of the sketch when it starts up.
//     For example:
//
//       MLX90614 not found           <== Only if not connected
//       MAC: 52-C4-58-43-37-4D       <== Randomly generated
//       No DHCP                      <== Only if no DHCP server gave an IP
//       IP: 169.254.211.176          <== 169.254.*.* only if no DHCP
//
//     It will check if there is a MLX90614 correctly hooked up to SDA, SCL,
//     5V and GND; if not, it will print "MLX90614 not found".
//     Then it will print the MAC address that it has (partially) randomly
//     generated (the first 6 of the hexadecimal digits are fixed by this
//     sketch).
//     It will attempt to allocate an IP address via requesting one from a
//     DHCP over the Ethernet. If such a server is not found, it will print
//     "No DHCP" and will print the IP address that it has randomly generated
//     from what is called the Link-Local address range (169.254.*.*). However,
//     if such a server does allocate an IP address to the Arduino, then it
//     will print that IP address.
//     Here is an example from the same board, but when "cleanly" starting:
//
//       MAC: 52-C4-58-43-37-4D
//       IP: 192.168.86.48
//
// 12) Confirm that the device is on your network by pinging the IP address
//     that it printed. For example:
//
//       $ ping 192.168.86.48
//       PING 192.168.86.48 (192.168.86.48) 56(84) bytes of data.
//       64 bytes from 192.168.86.48: icmp_seq=1 ttl=128 time=0.292 ms
//       64 bytes from 192.168.86.48: icmp_seq=2 ttl=128 time=0.340 ms
//       64 bytes from 192.168.86.48: icmp_seq=3 ttl=128 time=0.271 ms
//       ^C
//       --- 192.168.86.48 ping statistics ---
//       3 packets transmitted, 3 received, 0% packet loss, time 2032ms
//       rtt min/avg/max/mdev = 0.271/0.301/0.340/0.028 ms
//
// 13) Confirm that the webserver is working by opening this URL in your
//     web browser:
//
//       http://<IP-address-of-your-Arduino>/
//     
//     Using the exampes above, that would be http://192.168.86.48/
//     You should see something like this:
//
//       {"relay":1, "object":67.30, "ambient":69.03}
//
//     The last two values only appear if the sketch has read those values from
//     an attached MLX90614.
//
// 14) The sketch also advertises itself (using Multicast DNS) as having this
//     hostname:
//
//       rainsensor.local
//
//     Confirm that your computer is able to resolve that by opening this
//     URL in your web browser:
//
//       http://rainsensor.local/
//
//     Ideally you'll see the same kind of result as you saw earlier in the
//     browser.
