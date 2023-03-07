#define SPOTIFY_CONFIG_JSON "/spotify_diy_config.json"

#define REFRESH_TOKEN_LABEL "refreshToken"

bool fetchConfigFile(char *refreshToken) {
  if (SPIFFS.exists(SPOTIFY_CONFIG_JSON)) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open(SPOTIFY_CONFIG_JSON, "r");
    if (configFile) {
      Serial.println("opened config file");
      StaticJsonDocument<512> json;
      DeserializationError error = deserializeJson(json, configFile);
      serializeJsonPretty(json, Serial);
      if (!error) {
        Serial.println("\nparsed json");

        strcpy(refreshToken, json[REFRESH_TOKEN_LABEL]);
        return true;

      } else {
        Serial.println("failed to load json config");
        return false;
      }
    }
  } else {
    Serial.println("Config file does not exist");
    return false;
  }
}

void saveConfigFile(char *refreshToken) {
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json[REFRESH_TOKEN_LABEL] = refreshToken;

  File configFile = SPIFFS.open(SPOTIFY_CONFIG_JSON, "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
}
