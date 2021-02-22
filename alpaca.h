// WBox2 alpaca interface


#ifndef _WBOX2_ALPACA_H_
#define _WBOX2_ALPACA_H_

/*
  Alpaca documentation:  https://github.com/ASCOMInitiative/ASCOMRemote/blob/master/Documentation/ASCOM%20Alpaca%20API%20Reference.pdf
*/


bool connected;
const String DeviceDescription=  "Weather sensor box with rain and skytemp";
const String DriverName=         "observingconditions"; // ASCOM name
const int    DriverVersion=          DRIVER_VERSION;
const String DriverInfo=         "AM_Weatherbox";
const String Description=        "AAVSO AM_WeatherBox, Mega 2560" ;
const String InterfaceVersion= "1";   // alpaca v1
const String DiscoveryPacket= "alpacadiscovery1"; // ends with the interface version
const String GUID=    "478a35f0-2510-48c8-b884-931ae733532e";
#define INSTANCE_NUMBER 0

const String SERVERNAME= "AM WeatherBox1 observing conditions"; //w"; //box1";
const String MFG= "Alan Slisky Telecope Shop";
const String MFG_VERSION= "0.2.2";
const String LOCATION= "Lincoln, MA";

#include <ASCOMAPICommon_rest.h> // https://github.com/gasilvis/ESP_Alpaca_common

// ObservingConditions handlers
void handleNotImplemented(void) {
    String message;
    uint32_t clientID = (uint32_t)server.arg("ClientID").toInt();
    uint32_t clientTransID = (uint32_t)server.arg("ClientTransactionID").toInt();
    StaticJsonDocument<JSON_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();
    jsonResponseBuilder( root, clientID, clientTransID, ++serverTransID, "", AE_notImplemented, "Property is not implemented" );    
    serializeJson(doc, message);
#ifdef DEBUG_ESP_HTTP_SERVER
DEBUG_OUTPUT.println( message );
#endif
    server.send(200, "application/json", message);
}

void handleAveragePeriod(void) {

  
}

#include <SparkFunMLX90614.h>
extern IRTherm irTherm;
void handleSkytemperatureGet(void) {
    String message;
    uint32_t clientID = (uint32_t)server.arg("ClientID").toInt();
    uint32_t clientTransID = (uint32_t)server.arg("ClientTransactionID").toInt();
    StaticJsonDocument<JSON_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();
    jsonResponseBuilder( root, clientID, clientTransID, ++serverTransID, "Description", AE_Success, "" ); 
    irTherm.read();   
    root["Value"]= String(irTherm.object(), 2);    
    serializeJson(doc, message);
#ifdef DEBUG_ESP_HTTP_SERVER
DEBUG_OUTPUT.println( message );
#endif
    server.send(200, "application/json", message);

    return ;
}


void alpaca_setup() {
   
   // management api
   server.on("/management/apiversions", handleAPIversions);
   server.on("/management/v1/configureddevices", handleAPIconfiguredDevices);
   server.on("/management/v1/description", handleAPIdescription);
   
   //Common ASCOM handlers
   String preUri = "/api/v"+ InterfaceVersion+ "/";
   preUri += DriverName;
   preUri += "/";
   preUri += INSTANCE_NUMBER;
   preUri += "/";
   server.on(preUri+"action",              HTTP_PUT, handleAction );
   server.on(preUri+"commandblind",        HTTP_PUT, handleCommandBlind );
   server.on(preUri+"commandbool",         HTTP_PUT, handleCommandBool );
   server.on(preUri+"commandstring",       HTTP_PUT, handleCommandString );
   server.on(preUri+"connected",           handleConnected );
   server.on(preUri+"description",         HTTP_GET, handleDescriptionGet );     
   server.on(preUri+"driverinfo",          HTTP_GET, handleDriverInfoGet );
   server.on(preUri+"driverversion",       HTTP_GET, handleDriverVersionGet );
   server.on(preUri+"interfaceversion",    HTTP_GET, handleInterfaceVersionGet );
   server.on(preUri+"name",                HTTP_GET, handleNameGet );
   server.on(preUri+"supportedactions",    HTTP_GET, handleSupportedActionsGet );

   //ObservingConditions
   //server.on(preUri+"averageperiod",       handleAverageperiod ); //   get/put 0.0, meaning no averaging
   
   server.on(preUri+"cloudcover",          handleNotImplemented );
   server.on(preUri+"dewpoint",            handleNotImplemented );
   server.on(preUri+"humidity",            handleNotImplemented );
   server.on(preUri+"pressure",            handleNotImplemented );
   // rainrate
   server.on(preUri+"skybrightness",       handleNotImplemented );
   server.on(preUri+"skyquality",          handleNotImplemented );
   server.on(preUri+"skytemperature",      HTTP_GET, handleSkytemperatureGet );
   server.on(preUri+"starfwhm",            handleNotImplemented );
   // temperature
   server.on(preUri+"winddirection",       handleNotImplemented );
   server.on(preUri+"windgust",            handleNotImplemented );
   server.on(preUri+"windspeed",           handleNotImplemented );
   // refresh
   // sensordescription
   // timesincelastupdate
   
}

void handleDiscovery( int udpBytesCount ) {
    char inBytes[64];
    String message;
 
    //Serial.printf("UDP: %i bytes received from %s:%i\n", udpBytesCount, Udp.remoteIP().toString().c_str(), Udp.remotePort() );

    // We've received a packet, read the data from it
    Udp.read( inBytes, udpBytesCount); // read the packet into the buffer
    //Is it for us ?
    char protocol[16];
    strncpy( protocol, (char*) inBytes, 16);
    Serial.println(protocol);
    if ( strncasecmp( DiscoveryPacket.c_str(), protocol, 16 ) == 0 )
    {
      StaticJsonDocument<JSON_SIZE> doc;
      JsonObject root = doc.to<JsonObject>();
      Serial.println("responding");
      Udp.beginPacket( Udp.remoteIP(), Udp.remotePort() );
      //Respond with discovery message
      root["AlpacaPort"] = ALPACA_PORT;
      //na  String LocalIP = String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + 
      //                            WiFi.localIP()[2] + "." + WiFi.localIP()[3];
      //na root["IPAddress"] = LocalIP.c_str();
      root["Type"] = DriverName;
      //na root["Name"] = WiFi.hostname();
      //na root["UniqueID"] = system_get_chip_id();
      serializeJson(doc, message);
      Udp.write( message.c_str(), strlen(message.c_str()) * sizeof(char) );
      Udp.endPacket();   
      Serial.println(message.c_str());
    }
 }


#endif
