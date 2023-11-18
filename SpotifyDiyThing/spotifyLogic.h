
// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

String lastTrackUri;

SpotifyDisplay* sp_Display;

// so we can store the song name and artist name
char *songName;
char *songArtist;

SpotifyArduino spotify(client, NULL, NULL);

boolean albumArtChanged = false;

long songStartMillis;
long songDuration;

// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 5000; // Time between requests (5 seconds)
unsigned long requestDueTime;               // time when request due

unsigned long delayBetweenProgressUpdates = 500; // Time between requests (0.5 seconds)
unsigned long progressDueTime;               // time when request due

void spotifySetup(SpotifyDisplay* theDisplay, const char *clientId, const char *clientSecret) {
  sp_Display = theDisplay;
  client.setCACert(spotify_server_cert);
  spotify.lateInit(clientId, clientSecret);

}

void spotifyRefreshToken(const char *refreshToken) {
  spotify.setRefreshToken(refreshToken);

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in SpotifyArduino.h

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}

void handleCurrentlyPlaying(CurrentlyPlaying currentlyPlaying) {

  //printCurrentlyPlayingToSerial(currentlyPlaying);

  String newTrackUri = String(currentlyPlaying.trackUri);
  if (newTrackUri != lastTrackUri) {
    lastTrackUri = newTrackUri;

    // We have a new Song, need to update the text
    sp_Display->printCurrentlyPlayingToScreen(currentlyPlaying);

  }

  albumArtChanged = sp_Display->processImageInfo(currentlyPlaying);

  sp_Display->displayTrackProgress(currentlyPlaying.progressMs, currentlyPlaying.durationMs);

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

void updateProgressBar() {
  if (songStartMillis != 0 && millis() > progressDueTime) {
    long songProgress = millis() - songStartMillis;
    if (songProgress > songDuration) {
      songProgress = songDuration;
    }
    sp_Display->displayTrackProgress(songProgress, songDuration);
    progressDueTime = millis() + delayBetweenProgressUpdates;
  }
}

void updateCurrentlyPlaying(boolean forceUpdate) {
  if (forceUpdate || millis() > requestDueTime)
  {
    //Serial.print("Free Heap: ");
    //Serial.println(ESP.getFreeHeap());

    Serial.println("getting currently playing song:");
    // Check if music is playing currently on the account.
    int status = spotify.getCurrentlyPlaying(handleCurrentlyPlaying, SPOTIFY_MARKET);
    if (status == 200)
    {
      Serial.println("Successfully got currently playing");
      if (albumArtChanged)
      {
        sp_Display->clearImage();
        int displayImageResult = sp_Display->displayImage();

        if (displayImageResult)
        {
          albumArtChanged = false;
        }
        else
        {
          Serial.print("failed to display image: ");
          Serial.println(displayImageResult);
        }
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
