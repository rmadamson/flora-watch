#include <Adafruit_NeoPixel.h>

#define PIN 12
#define NUMPIXELS 12
#define BRIGHTNESS 30

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {

//  while(!Serial);
//    delay(500);
//  Serial.begin(9600);
//  Serial.println(F("Started"));

  for(uint32_t t = 0; t < 2000000; t++) {
    displaytime( t );
    delay(5);
  }
  
}

void displaytime( uint32_t t ) {

  uint8_t red[NUMPIXELS];
  uint8_t green[NUMPIXELS];
  uint8_t blue[NUMPIXELS];
  
  memset( red, 0, sizeof(red) );
  memset( green, 0, sizeof(green) );
  memset( blue, 0, sizeof(blue) );
  
  uint16_t seconds = t % 60;
  uint16_t minutes = t % (60 * 60);
  uint16_t hours   = t % (60 * 60 * 12);

  uint16_t numbuckets  = NUMPIXELS;
  
  sethand( red,   hours,   numbuckets, 60*60*12/numbuckets );
  sethand( green, minutes, numbuckets, 60*60/numbuckets );
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

