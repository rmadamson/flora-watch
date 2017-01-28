#include <Adafruit_NeoPixel.h>
#include <Adafruit_GPS.h>

//#define DEBUG 1

#define PIN 12
#define NUMPIXELS 12
#define BRIGHTNESS 100
#define SKEWTIME 10000
#define SKEW_UPDATE_FREQUENCY 10000
#define DISPLAY_UPDATE_FREQUENCY 10
#define MAXSKEW 100 //SKEW_UPDATE_FREQUENCY / DISPLAY_UPDATE_FREQUENCY

#define MILLISECONDSINSECOND 1000
#define SECONDSINMINUTE      60
#define MINUTESINHOUR        60
#define HOURSINCLOCK         12



Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
uint32_t skewupdatetimer = millis();
uint32_t displayupdatetimer = skewupdatetimer;
uint32_t globaltime = 0;
uint32_t previoustime = 0;
int32_t skew = 0;

// This is from the neopixel goggles example.  Correct colors
// and boost midrange ones.
const uint8_t gamma[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
      1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
      3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
      7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
     13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
     20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
     30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
     42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
     58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
     76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
     97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
    122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
    150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
    182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
    218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255
};



////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

#ifdef DEBUG
  Serial.begin(115200);
  while(!Serial);
    delay(500);
#endif

  delay(2000);
  read_gps_data(5000);
  delay(2000);
  read_gps_data(5000);

  
  if( GPS.fix ) {
      globaltime = convert_hms_to_timestamp( GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds );
  }

}

////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

#ifdef DEBUG
  Serial.println(F("\n\n--- Main Loop ---"));
  Serial.print(F("  prevtime: ")); Serial.println(previoustime);
#endif

  read_gps_data( 100 ); // Read from the gps serial connection and parse
  uint32_t timestamp = millis();
  globaltime += timestamp - previoustime;
  previoustime = timestamp;

  if( skewupdatetimer > timestamp ) skewupdatetimer = timestamp; // Handle timer wraps
  if( displayupdatetimer > timestamp ) displayupdatetimer = timestamp; // Handle timer wraps

  if( timestamp - skewupdatetimer > SKEW_UPDATE_FREQUENCY ) { // Every so many millseconds, update time with gps time
    #ifdef DEBUG
    Serial.print(F("globaltime: ")); Serial.println(globaltime);
    Serial.print(F("      skew: ")); Serial.println(skew);
    Serial.print(F(" timestamp: ")); Serial.println(timestamp);
    Serial.print(F(" fix: ")); Serial.println(GPS.fix);
    #endif
    if( GPS.fix ) { set_gps_skew(); }
    skewupdatetimer = timestamp;
  }

  if( timestamp - displayupdatetimer > ( DISPLAY_UPDATE_FREQUENCY ) ) { // Every so many millseconds, redo computation
    globaltime += skew; // Do skew in a rate-limited way
    displaytime();
    displayupdatetimer = timestamp;
  }

}



////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void read_gps_data( uint16_t characters ) {
  for( uint16_t i=0; i < characters; i++ ) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
      if (!GPS.parse(GPS.lastNMEA())) continue;
    } else {
      return;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

uint32_t convert_hms_to_timestamp( uint32_t hours, uint32_t minutes, uint32_t seconds, uint32_t milliseconds ) {

  return uint32_t(milliseconds) + 1000 * (uint32_t(seconds) + 60 * (uint32_t(minutes) + 60 * (uint32_t(hours) % 24) ) );
 
}



////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void convert_timestamp_to_hms( uint32_t &hours, uint32_t &minutes, uint32_t &seconds, uint32_t &milliseconds, uint32_t timestamp ) {

  hours = timestamp / 1000;
  hours = hours / 60;
  hours = hours / 60;
  timestamp = timestamp - (hours * 1000 * 60 * 60); // Chop the hour hand off of our milliseconds

  minutes = timestamp / 1000;
  minutes = minutes / 60;
  timestamp = timestamp - (minutes * 1000 * 60);

  seconds = timestamp / 1000;
  timestamp = timestamp - (seconds * 1000);
  
  milliseconds = timestamp;

}



////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void set_gps_skew() {

    // Adjust hours for daylight savings time and GMT-500
    uint32_t gpstime = convert_hms_to_timestamp( GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds );
    skew = int32_t(gpstime) - int32_t(globaltime) / SKEW_UPDATE_FREQUENCY; // Set skew so that it converges when we would check next.
    
#ifdef DEBUG
   Serial.println(F("--- Set Skew (pre) ---"));
   Serial.print("\nGPS Reported Time: ");
   Serial.print(GPS.hour, DEC); Serial.print(':');
   Serial.print(GPS.minute, DEC); Serial.print(':');
   Serial.print(GPS.seconds, DEC); Serial.print('.');
   Serial.println(GPS.milliseconds);
   
   Serial.print(F("   GPSTime: ")); Serial.println(gpstime);
   Serial.print(F("GlobalTime: ")); Serial.println(globaltime);
   Serial.print(F("      Skew: ")); Serial.println(skew);
#endif

    if( skew > MAXSKEW ) { // If we would be 'speeding up time' by more than MAXSKEW, just go ahead and force set the time
      globaltime = gpstime;
      skew = 0;
    }

#ifdef DEBUG
   Serial.println(F("--- Set Skew (post) ---"));
   Serial.print("\nGPS Reported Time: ");
   Serial.print(GPS.hour, DEC); Serial.print(':');
   Serial.print(GPS.minute, DEC); Serial.print(':');
   Serial.print(GPS.seconds, DEC); Serial.print('.');
   Serial.println(GPS.milliseconds);
   
   Serial.print(F("   GPSTime: ")); Serial.println(gpstime);
   Serial.print(F("GlobalTime: ")); Serial.println(globaltime);
   Serial.print(F("      Skew: ")); Serial.println(skew);
#endif
  
}

////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

void displaytime() {

   uint32_t hours = 0;
   uint32_t minutes = 0;
   uint32_t seconds = 0;
   uint32_t milliseconds = 0;

   convert_timestamp_to_hms( hours, minutes, seconds, milliseconds, globaltime );
    
#ifdef DEBUG
  Serial.println(F("--- Displaytime ---"));
  Serial.print("Time from GPS: ");
  Serial.print(GPS.hour, DEC); Serial.print(':');
  Serial.print(GPS.minute, DEC); Serial.print(':');
  Serial.print(GPS.seconds, DEC); Serial.print('.');
  Serial.print(GPS.milliseconds, DEC); Serial.println('.');

  Serial.println(F("Displaying Time: hh:mm:ss.ms."));
  Serial.print( hours );  Serial.print(':');
  Serial.print( minutes ); Serial.print(':');
  Serial.print( seconds ); Serial.print('.');
  Serial.print( milliseconds ); Serial.println('.');
#endif

  uint8_t red[NUMPIXELS];
  uint8_t green[NUMPIXELS];
  uint8_t blue[NUMPIXELS];
  
  memset( red,   0, sizeof(red) );
  memset( green, 0, sizeof(green) );
  memset( blue,  0, sizeof(blue) );

  uint32_t bucket = seconds / (60 / NUMPIXELS);
  
  float percentofbucketelapsed = seconds % 5;
  percentofbucketelapsed = percentofbucketelapsed * 1000;
  percentofbucketelapsed = percentofbucketelapsed + float(milliseconds);
  percentofbucketelapsed = percentofbucketelapsed / 5000;
  blue[bucket % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * (1-percentofbucketelapsed) );
  blue[(bucket + 1) % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * percentofbucketelapsed );

  bucket = minutes / (60 / NUMPIXELS);
  percentofbucketelapsed = minutes % 5;
  percentofbucketelapsed = percentofbucketelapsed * 60;
  percentofbucketelapsed = percentofbucketelapsed + float(seconds);
  percentofbucketelapsed = percentofbucketelapsed / 300;
  green[bucket % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * (1-percentofbucketelapsed) );
  green[(bucket + 1) % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * percentofbucketelapsed );

  bucket = hours / (12 / NUMPIXELS) - 5; // adjust for GMT
  percentofbucketelapsed = float(minutes);
  percentofbucketelapsed = percentofbucketelapsed / 60;
  red[bucket % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * (1-percentofbucketelapsed));
  red[(bucket+1) % NUMPIXELS] = uint8_t( float(BRIGHTNESS) * percentofbucketelapsed );

  for( uint32_t i=0; i < NUMPIXELS; i++ ) {
    strip.setPixelColor(i, strip.Color(gamma[red[i]], gamma[green[i]], gamma[blue[i]]));
    strip.show();
  }

}
