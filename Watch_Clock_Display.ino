#include <Adafruit_NeoPixel.h>

#define PIN 12
#define NUMPIXELS 12
#define BRIGHTNESS 20

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // put your main code here, to run repeatedly:

  //while(!Serial);
    //delay(500);

//  Serial.begin(9600);
//  Serial.println(F("Started"));

  for(uint32_t t = 0; t < 2000000; t++) {
    displaytime( t );
    delay(5);
  }
  
}

void displaytime( uint32_t t ) {

  uint8_t color[NUMPIXELS][3];
  memset( color, 0, sizeof(color) );
  
  uint16_t seconds = t % 60;
  uint16_t minutes = t % (60 * 60);
  uint16_t hours   = t % (60 * 60 * 12);

  uint16_t numbuckets  = NUMPIXELS;
  uint16_t bucketwidth = 60 / numbuckets;

  // Let's figure out seconds
  uint16_t secondsbucket = seconds / bucketwidth;
  float secondspower = float(seconds - (secondsbucket * bucketwidth)) / float(bucketwidth);
  
  color[secondsbucket % numbuckets][2] = float(BRIGHTNESS) * (1-(secondspower*secondspower*secondspower));
  color[(secondsbucket + 1) % numbuckets][2] = float(BRIGHTNESS) * secondspower*secondspower*secondspower;


  bucketwidth = 60*60 / numbuckets;
  uint16_t minutesbucket = minutes / bucketwidth;
  float minutespower = float(minutes - (minutesbucket * bucketwidth)) / float(bucketwidth);
  
  color[minutesbucket][1] = float(BRIGHTNESS) * (1-(minutespower*minutespower*minutespower));
  color[(minutesbucket + 1) % numbuckets][1] = float(BRIGHTNESS) * minutespower*minutespower*minutespower;


  bucketwidth = 60*60*12 / numbuckets;
  uint16_t hoursbucket = hours / bucketwidth;
  float hourspower = float(hours - (hoursbucket * bucketwidth)) / float(bucketwidth);
  
  color[hoursbucket][0] = float(BRIGHTNESS) * (1-(hourspower*hourspower*hourspower));
  color[(hoursbucket + 1) % numbuckets][0] = float(BRIGHTNESS) * hourspower*hourspower*hourspower;

 // Serial.print(F("Time: "));
 // Serial.println(t);
 // Serial.println(minutespower);
 // Serial.println((1-(minutespower*minutespower*minutespower)));
 // Serial.println(((minutespower*minutespower*minutespower)));


//  Serial.print(F("Seconds:        "));
//  Serial.println(seconds);
//  Serial.print(F("Bucketwidth:    "));
//  Serial.println(bucketwidth);
  
//  Serial.print(F("Secondsbucket:  "));
//  Serial.println(secondsbucket);

//  Serial.print(F("Secondspower:   "));
//  Serial.println(secondspower);
  

  
  for( uint16_t i=0; i < numbuckets; i++ ) {
    strip.setPixelColor(i, strip.Color(color[i][0], color[i][1], color[i][2]));
  }
  
  strip.show();
  
}
