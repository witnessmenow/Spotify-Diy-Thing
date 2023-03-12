/*******************************************************************
    Displays Album Art on a 320 x 240 ESP32.

    Parts:
    ESP32 With Built in 320x240 LCD with Touch Screen (ESP32-2432S028R)
    https://github.com/witnessmenow/Spotify-Diy-Thing#hardware-required

 *******************************************************************/

// ----------------------------
// Library Defines - Need to be defined before library import
// ----------------------------

#define ESP_DRD_USE_SPIFFS      true

// ----------------------------
// Standard Libraries
// ----------------------------
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <FS.h>
#include "SPIFFS.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <WiFiManager.h>
// Captive portal for configuring the WiFi

// If installing from the library manager (Search for "WifiManager")
// https://github.com/tzapu/WiFiManager

#include <ESP_DoubleResetDetector.h>
// A library for checking if the reset button has been pressed twice
// Can be used to enable config mode
// Can be installed from the library manager (Search for "ESP_DoubleResetDetector")
//https://github.com/khoih-prog/ESP_DoubleResetDetector

#include <TFT_eSPI.h>
// A library for checking if the reset button has been pressed twice
// Can be used to enable config mode
// Can be installed from the library manager (Search for "ESP_DoubleResetDetector")
//https://github.com/khoih-prog/ESP_DoubleResetDetector

#include <JPEGDEC.h>
// Library for decoding Jpegs from the API responses

#include <SpotifyArduino.h>
#include <ArduinoJson.h>


// ----------------------------
// Internal includes
// ----------------------------

#include "displayCode.h"

#include "touchScreen.h"

#include "configFile.h"

#include "refreshToken.h"

#include "serialPrint.h"

#include "WifiManager.h"

//------- Replace the following! ------

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"
//------- ---------------------- ------


// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

String lastTrackUri;

// Variable to hold image info
SpotifyImage medImage;

// so we can store the song name and artist name
char *songName;
char *songArtist;

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret);

// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 5000; // Time between requests (5 seconds)
unsigned long requestDueTime;               // time when request due

unsigned long delayBetweenProgressUpdates = 500; // Time between requests (0.5 seconds)
unsigned long progressDueTime;               // time when request due

unsigned long touchScreenCoolDownInterval = 200;  // How long after a touch press do we accept another (0.2 seconds). There is also an APi request inbetween
unsigned long touchScreenCoolDownTime;               // time when cool down has expired

long songStartMillis;
long songDuration;

void setup()
{
  Serial.begin(115200);

  bool forceConfig = false;

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset()) {
    Serial.println(F("Forcing config mode as there was a Double reset detected"));
    forceConfig = true;
  }

  displaySetup(&spotify);
  touchSetup(&spotify);

  // Initialise SPIFFS, if this fails try .begin(true)
  // NOTE: I believe this formats it though it will erase everything on
  // spiffs already! In this example that is not a problem.
  // I have found once I used the true flag once, I could use it
  // without the true flag after that.
  bool spiffsInitSuccess = SPIFFS.begin(false) || SPIFFS.begin(true);
  if (!spiffsInitSuccess)
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  //  WiFi.mode(WIFI_STA);
  //  WiFi.begin(ssid, password);
  //  Serial.println("");
  //
  //  // Wait for connection
  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println("");
  //  Serial.print("Connected to ");
  //  Serial.println(ssid);
  //  Serial.print("IP address: ");
  //  Serial.println(WiFi.localIP());

  client.setCACert(spotify_server_cert);

  pinMode(0, INPUT); //has an internal pullup
  refreshToken[0] = '\0';
  if (!fetchConfigFile(refreshToken, clientId, clientSecret)) {
    // Failed to fetch config file, need to launch Wifi Manager
    forceConfig = true;
  }

  setupWiFiManager(forceConfig, refreshToken, &saveConfigFile, &drawWifiManagerMessage);

  // If we are here we should be connected to the Wifi
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  spotify.lateInit(clientId, clientSecret);

  // Check if we have a refresh Token
  if (digitalRead(0) == LOW || refreshToken[0] == '\0') {
    drawRefreshTokenMessage();
    Serial.println("Launching refresh token flow");
    if (launchRefreshTokenFlow(&spotify, clientId)) {
      Serial.print("Refresh Token: ");
      Serial.println(refreshToken);
      saveConfigFile(refreshToken, clientId, clientSecret);
    }
  }

  spotify.setRefreshToken(refreshToken);

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
  tft.fillScreen(TFT_BLACK);
  drawTouchButtons(false, false);
}

void getNewAlbumArt(CurrentlyPlaying currentlyPlaying) {
  medImage = currentlyPlaying.albumImages[currentlyPlaying.numImages - 2];
}

void handleCurrentlyPlaying(CurrentlyPlaying currentlyPlaying) {

  //printCurrentlyPlayingToSerial(currentlyPlaying);

  String newTrackUri = String(currentlyPlaying.trackUri);
  if (newTrackUri != lastTrackUri) {
    lastTrackUri = newTrackUri;

    // We have a new Song, need to update the text
    printCurrentlyPlayingToScreen(currentlyPlaying);

  }

  getNewAlbumArt(currentlyPlaying);

  displayTrackProgress(currentlyPlaying.progressMs, currentlyPlaying.durationMs);

  if (currentlyPlaying.isPlaying) {
    // If we know at what millis the song started at, we can make a good guess
    // at updating the progress bar more often than checking the API
    songStartMillis = millis() - currentlyPlaying.progressMs;
    songDuration = currentlyPlaying.durationMs;
  } else {
    // Song doesn't seem to be playing, do not update the progress
    songStartMillis = 0;
  }

}

void loop()
{
  drd->loop();
  if (millis() > requestDueTime)
  {
    //Serial.print("Free Heap: ");
    //Serial.println(ESP.getFreeHeap());

    Serial.println("getting currently playing song:");
    // Check if music is playing currently on the account.
    int status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
    if (status == 200)
    {
      Serial.println("Successfully got currently playing");
      String newAlbum = String(medImage.url);
      if (newAlbum != lastAlbumArtUrl)
      {
        setImageSize(medImage);
        Serial.println("Updating Art");
        clearImage();
        char *my_url = const_cast<char *>(medImage.url);
        int displayImageResult = displayImageUsingFile(my_url);

        if (displayImageResult == 1)
        {
          lastAlbumArtUrl = newAlbum;
        }
        else
        {
          Serial.print("failed to display image: ");
          Serial.println(displayImageResult);
        }
      }
      else if (status == 204)
      {
        songStartMillis = 0;
        Serial.println("Doesn't seem to be anything playing");
      }
      else
      {
        Serial.print("Error: ");
        Serial.println(status);
      }

      requestDueTime = millis() + delayBetweenRequests;
    }
  }
  if (songStartMillis != 0 && millis() > progressDueTime) {
    long songProgress = millis() - songStartMillis;
    if (songProgress > songDuration) {
      songProgress = songDuration;
    }
    displayTrackProgress(songProgress, songDuration);
    progressDueTime = millis() + delayBetweenProgressUpdates;
  }

  if (millis() > touchScreenCoolDownTime && handleTouched()) {
    drawTouchButtons(previousTrackStatus, nextTrackStatus);
    if (previousTrackStatus) {
      spotify.previousTrack();
    } else if (nextTrackStatus) {
      spotify.nextTrack();
    }
    drawTouchButtons(false, false);
    requestDueTime = 0; //Some button has been pressed and acted on, it surely impacts the status so force a refresh
    touchScreenCoolDownTime  = millis() + touchScreenCoolDownInterval; //Cool the touch off
  }
}
