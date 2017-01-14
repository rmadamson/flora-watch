#include <Adafruit_NeoPixel.h>
#include <Adafruit_GPS.h>


#define PIN 12
#define NUMPIXELS 12
#define BRIGHTNESS 30

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
uint32_t timer = millis();

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  
//  while(!Serial);
//    delay(500);
//  Serial.begin(9600);
//  Serial.println(F("Started"));

}

void loop() {

//  for(uint32_t t = 0; t < 2000000; t++) {
//    displaytime( t );
//    delay(5);
//  }
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    // Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  if( timer > millis() ) timer = millis(); // Handle timer wraps
  if( millis()-timer > 1000 ) {
    uint8_t hours = GPS.hour%12;
    uint8_t minutes = GPS.minute;
    uint8_t seconds = GPS.seconds;
    displaytime( hours, minutes, seconds );
    timer = millis();
  }

}

void displaytime( uint16_t hours, uint16_t minutes, uint16_t seconds ) {

  // Adjust for timezone
  hours = (hours - 4) % 12;
  //Serial.println("---");
  //Serial.println( hours );
  //Serial.println( minutes );
  //Serial.println( seconds );
  
  uint8_t red[NUMPIXELS];
  uint8_t green[NUMPIXELS];
  uint8_t blue[NUMPIXELS];
  
  memset( red, 0, sizeof(red) );
  memset( green, 0, sizeof(green) );
  memset( blue, 0, sizeof(blue) );
  
  uint16_t numbuckets  = NUMPIXELS;
  
  sethand( red,   hours*60*12,   numbuckets, 60*60*12/numbuckets );
  sethand( green, minutes*60, numbuckets, 60*60/numbuckets );
  sethand( blue,  seconds, numbuckets, 60/numbuckets );

  for( uint16_t i=0; i < numbuckets; i++ ) {
    strip.setPixelColor(i, strip.Color(red[i], green[i], blue[i]));
  }
  
  strip.show();
}


void sethand( uint8_t *color, uint16_t units, uint16_t numbuckets, uint16_t bucketwidth ) {

  uint16_t bucket = units / bucketwidth;
  float power = float(units - (bucket * bucketwidth)) / float(bucketwidth);
  
  color[bucket % numbuckets] = float(BRIGHTNESS) * (1-(power*power));
  color[(bucket + 1) % numbuckets] = float(BRIGHTNESS)*power*power;

}

