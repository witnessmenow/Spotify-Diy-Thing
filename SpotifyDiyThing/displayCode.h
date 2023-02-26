/*******************************************************************
    All code to do with displaying on the display

 *******************************************************************/

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

int imageSize = 150;
int screenWidth = 320;
int screenHeight = 240;

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

SpotifyArduino *spotify_display;

void displaySetup(SpotifyArduino *spotifyObj) {
  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  spotify_display = spotifyObj;
}

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

int displayImage(char *imageFileUri) {
  unsigned long lTime = millis();
  lTime = millis();
  jpeg.open((const char *) imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
  jpeg.setPixelType(1);
  int imagePosition = (screenWidth / 2) - (imageSize / 2);
  // decode will return 1 on sucess and 0 on a failure
  int decodeStatus = jpeg.decode(imagePosition, 0, JPEG_SCALE_HALF);
  //jpeg.decode(45, 0, 0);
  jpeg.close();
  Serial.print("Time taken to decode and display Image (ms): ");
  Serial.println(millis() - lTime);

  return decodeStatus;
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

  bool gotImage = spotify_display->getImage(albumArtUrl, &f);

  // Make sure to close the file!
  f.close();

  if (gotImage)
  {
    return displayImage(ALBUM_ART);
  }
  else
  {
    return -2;
  }
}

void clearImage() {
  int imagePosition = (screenWidth / 2) - (imageSize / 2);
  tft.fillRect(imagePosition, 0, imageSize, imageSize, TFT_BLACK);
}

void printCurrentlyPlayingToScreen(CurrentlyPlaying currentlyPlaying) {
  // Clear the song area
  int textStartY = imageSize + 30;
  int screenCenter = screenWidth / 2;
  tft.fillRect(0, textStartY, screenWidth, screenHeight - textStartY, TFT_BLACK);

  tft.drawCentreString(currentlyPlaying.trackName, screenCenter, textStartY, 2);
  tft.drawCentreString(currentlyPlaying.artists[0].artistName, screenCenter, textStartY + 18, 2);
  tft.drawCentreString(currentlyPlaying.albumName, screenCenter, textStartY + 36, 2);
}

void displayTrackProgress(long progress, long duration) {

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
