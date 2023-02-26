#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass mySpi = SPIClass(HSPI);

XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

SpotifyArduino *spotify_touch;

void touchSetup(SpotifyArduino *spotifyObj) {
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
  spotify_touch = spotifyObj;
}

bool handleTouched(){
  if (ts.tirqTouched() && ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    delay(30);
    Serial.println();
    if(p.x < 1000){
      spotify_touch->previousTrack();
      return true;
    } else if(p.x > 2800){
      spotify_touch->nextTrack();
      return true;
    }
  }
  
  return false;
  
}
