/*******************************************************************
    Displays Album Art on a 320 x 240 ESP32.

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    ESP32 With Built in 320x240 LCD with Touch Screen (ESP32-2432S028R)
    https://github.com/witnessmenow/Spotify-Diy-Thing#hardware-required

 *******************************************************************/

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

#include <TFT_eSPI.h>

#include <JPEGDEC.h>
// Library for decoding Jpegs from the API responses


#include <SpotifyArduino.h>
#include <ArduinoJson.h>

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

char clientId[] = "56t4373258u3405u43u543";     // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)

// Country code, including this is advisable
#define SPOTIFY_MARKET "IE"

#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

//------- ---------------------- ------

// including a "spotify_server_cert" variable
// header is included as part of the SpotifyArduino libary
#include <SpotifyArduinoCert.h>

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

int imageSize = 150;
int screenWidth = 320;
int screenHeight = 240;

// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

String lastTrackUri;

// Variable to hold image info
SpotifyImage medImage;

// so we can store the song name and artist name
char *songName;
char *songArtist;

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);

// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 5000; // Time between requests (5 seconds)
unsigned long requestDueTime;               // time when request due

unsigned long delayBetweenProgressUpdates = 500; // Time between requests (0.5 seconds)
unsigned long progressDueTime;               // time when request due

long songStartMillis;
long songDuration;

TFT_eSPI tft = TFT_eSPI();

JPEGDEC jpeg;

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (  pDraw->y >= tft.height() ) return 0;

  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
//bool displayOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
//{
//    // Stop further decoding as image is running off bottom of screen
//    if (y >= tft.height())
//        return 0;
//
//    tft.pushImage(x, y, w, h, bitmap);
//
//    // Return 1 to decode next block
//    return 1;
//}

void setup()
{
  Serial.begin(115200);

  // Initialise SPIFFS, if this fails try .begin(true)
  // NOTE: I believe this formats it though it will erase everything on
  // spiffs already! In this example that is not a problem.
  // I have found once I used the true flag once, I could use it
  // without the true flag after that.

  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setCACert(spotify_server_cert);

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}

int displayImageUsingFile(char *albumArtUrl)
{

  // In this example I reuse the same filename
  // over and over, maybe saving the art using
  // the album URI as the name would be better
  // as you could save having to download them each
  // time, but this seems to work fine.
  if (SPIFFS.exists(ALBUM_ART) == true)
  {
    Serial.println("Removing existing image");
    SPIFFS.remove(ALBUM_ART);
  }

  fs::File f = SPIFFS.open(ALBUM_ART, "w+");
  if (!f)
  {
    Serial.println("file open failed");
    return -1;
  }

  bool gotImage = spotify.getImage(albumArtUrl, &f);

  // Make sure to close the file!
  f.close();

  if (gotImage)
  {
    displayImage(ALBUM_ART);
    return 0;
  }
  else
  {
    return -2;
  }
}

fs::File myfile;

void * myOpen(const char *filename, int32_t *size) {
  myfile = SPIFFS.open(filename);
  *size = myfile.size();
  return &myfile;
}
void myClose(void *handle) {
  if (myfile) myfile.close();
}
int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
int32_t mySeek(JPEGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

void displayImage(char *imageFileUri) {
  unsigned long lTime = millis();
  lTime = millis();
  jpeg.open((const char *) imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
  jpeg.setPixelType(1);
  int imagePosition = (screenWidth/2) - (imageSize/2);
  jpeg.decode(imagePosition, 0, JPEG_SCALE_HALF);
  //jpeg.decode(45, 0, 0);
  jpeg.close();
  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);
}

void clearImage(){
  int imagePosition = (screenWidth/2) - (imageSize/2);
  tft.fillRect(imagePosition, 0, imageSize, imageSize, TFT_BLACK);
}

void printCurrentlyPlayingToScreen(CurrentlyPlaying currentlyPlaying){
  // Clear the song area
  int textStartY = imageSize + 30;
  int screenCenter = screenWidth/2;
  tft.fillRect(0, textStartY, screenWidth, screenHeight - textStartY, TFT_BLACK);

  tft.drawCentreString(currentlyPlaying.trackName, screenCenter, textStartY, 2);
  tft.drawCentreString(currentlyPlaying.artists[0].artistName, screenCenter, textStartY + 18, 2);
  tft.drawCentreString(currentlyPlaying.albumName, screenCenter, textStartY + 36, 2);
}

void getNewAlbumArt(CurrentlyPlaying currentlyPlaying){
  medImage = currentlyPlaying.albumImages[currentlyPlaying.numImages - 2];
}

void displayTrackProgress(long progress, long duration){

  Serial.print("Elapsed time of song (ms): ");
  Serial.print(progress);
  Serial.print(" of ");
  Serial.println(duration);
  Serial.println();

  float percentage = ((float)progress / (float)duration) * 100;
  int clampedPercentage = (int)percentage;
  Serial.println(clampedPercentage);
  int barXWidth = map(clampedPercentage, 0, 100, 0, screenWidth - 40);
  Serial.println(barXWidth);
  
  tft.drawRect(19, imageSize + 5, screenWidth - 38, 20, TFT_WHITE);
  tft.fillRect(20, imageSize + 6, barXWidth, 18, TFT_WHITE);
  tft.fillRect(20 + barXWidth, imageSize + 6, (screenWidth - 20) - (20 + barXWidth), 18, TFT_BLACK);

  
}

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
  // Use the details in this method or if you want to store them
  // make sure you copy them (using something like strncpy)
  // const char* artist =

  // Clear the Text every time a new song is created
  Serial.println("--------- Currently Playing ---------");

  Serial.print("Is Playing: ");
  if (currentlyPlaying.isPlaying)
  {
    Serial.println("Yes");
  }
  else
  {
    Serial.println("No");
  }

  Serial.print("Track: ");
  Serial.println(currentlyPlaying.trackName);
  // Save the song name to a variable
  songName = const_cast<char *>(currentlyPlaying.trackName);
  //drawMessage(0, 160, songName);
  Serial.print("Track URI: ");
  Serial.println(currentlyPlaying.trackUri);
  Serial.println();

  Serial.println("Artists: ");
  for (int i = 0; i < currentlyPlaying.numArtists; i++)
  {
    Serial.print("Name: ");
    // Save the song artist name to a variable
    Serial.println(currentlyPlaying.artists[i].artistName);
    songArtist = const_cast<char *>(currentlyPlaying.artists[0].artistName);
    //drawMessage(0, 170, songArtist);
    Serial.print("Artist URI: ");
    Serial.println(currentlyPlaying.artists[i].artistUri);
    Serial.println();
  }

  Serial.print("Album: ");
  Serial.println(currentlyPlaying.albumName);
  Serial.print("Album URI: ");
  Serial.println(currentlyPlaying.albumUri);
  Serial.println();

  long progress = currentlyPlaying.progressMs; // duration passed in the song
  long duration = currentlyPlaying.durationMs; // Length of Song
  Serial.print("Elapsed time of song (ms): ");
  Serial.print(progress);
  Serial.print(" of ");
  Serial.println(duration);
  Serial.println();

  float percentage = ((float)progress / (float)duration) * 100;
  int clampedPercentage = (int)percentage;
  Serial.print("<");
  for (int j = 0; j < 50; j++)
  {
    if (clampedPercentage >= (j * 2))
    {
      Serial.print("=");
    }
    else
    {
      Serial.print("-");
    }
  }
  Serial.println(">");
  Serial.println();

  // will be in order of widest to narrowest
  // currentlyPlaying.numImages is the number of images that
  // are stored

  for (int i = 0; i < currentlyPlaying.numImages; i++)
  {
    // Save the second album image into the smallestImage Variable above.
    Serial.println("------------------------");
    Serial.print("Album Image: ");
    Serial.println(currentlyPlaying.albumImages[i].url);
    Serial.print("Dimensions: ");
    Serial.print(currentlyPlaying.albumImages[i].width);
    Serial.print(" x ");
    Serial.print(currentlyPlaying.albumImages[i].height);
    Serial.println();
  }
  Serial.println("------------------------");
}

void handleCurrentlyPlaying(CurrentlyPlaying currentlyPlaying){
  String newTrackUri = String(currentlyPlaying.trackUri);
  if(newTrackUri != lastTrackUri){
    lastTrackUri = newTrackUri;
    
    // We have a new Song, need to update the text
    printCurrentlyPlayingToScreen(currentlyPlaying);
    
  }

  getNewAlbumArt(currentlyPlaying);

  songStartMillis = millis() - currentlyPlaying.progressMs;
  songDuration = currentlyPlaying.durationMs;
  displayTrackProgress(currentlyPlaying.progressMs, currentlyPlaying.durationMs);
}

void loop()
{
  if (millis() > requestDueTime)
  {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());

    Serial.println("getting currently playing song:");
    // Check if music is playing currently on the account.
    int status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
    if (status == 200)
    {
      Serial.println("Successfully got currently playing");
      String newAlbum = String(medImage.url);
      if (newAlbum != lastAlbumArtUrl)
      {
        Serial.println("Updating Art");
        clearImage();
        char *my_url = const_cast<char *>(medImage.url);
        int displayImageResult = displayImageUsingFile(my_url);

        if (displayImageResult == 0)
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
  if(songStartMillis != 0 && millis() > progressDueTime){
    long songProgress = millis() - songStartMillis;
    if(songProgress > songDuration){
      songProgress = songDuration;
    }
    displayTrackProgress(songProgress, songDuration);
    progressDueTime = millis() + delayBetweenProgressUpdates;
  }
}

// Method to draw messages at a certain point on a TFT Display.
void drawMessage(int x, int y, char *message)
{
  tft.setTextColor(TFT_WHITE);
  tft.drawString(message, x, y);
}
