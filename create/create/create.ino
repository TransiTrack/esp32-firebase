
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to create a document in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <TimeLib.h>  
// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Wil"
#define WIFI_PASSWORD "1223334444"


/* 2. Define the API Key */
#define API_KEY "AIzaSyDVi1awMUCpMK6P-AejbbG0Ga2jBtD13Vo"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "transitrack-709f1"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "zedec.pacaldo@gmail.com"
#define USER_PASSWORD "password123"

#define DEVICE_ID "jeep1"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long timestamp = 1671936000;
unsigned long dataMillis = 0;
int count = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

float coordinates[][2] = {
  {14.657675, 121.062360},
  {14.654756, 121.062316},
  {14.647361, 121.062336},
  {14.647706, 121.063844},
  {14.647659, 121.064632},
  {14.647939, 121.065780},
  {14.647960, 121.066328},
  {14.647254, 121.067808},
  {14.647173, 121.068955},
  {14.649071, 121.068951},
  {14.649904, 121.068611},
  {14.650504, 121.068453},
  {14.650908, 121.068430},
  {14.651842, 121.068584},
  {14.652487, 121.068667},
  {14.652550, 121.072847},
  {14.653974, 121.072828},
  {14.654645, 121.073132},
  {14.655566, 121.073090},
  {14.656308, 121.072771},
  {14.659379, 121.072722},
  {14.659390, 121.068572},
  {14.657539, 121.068584},
  {14.657568, 121.064787},
  {14.657675, 121.062360}
};

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void setup()
{

    Serial.begin(115200);
    // 2. Initialize WiFi and Time
    connectToWiFi();
    configTimeFromNTP();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    // 5. Initialize firebase connection
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    // Assign the callback function for the long running token generation task 
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth); // DONT FORGET TO IMPLMENT for case when FAIL
    Firebase.reconnectWiFi(true);
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0))
    {
      updateTimestamp();

      if (count > 23){
        count = 0;
      }

      dataMillis = millis();

      String collectionPath = "/jeeps/" + String(DEVICE_ID) + "/timeline";

      String documentPath = collectionPath + "/";

      FirebaseJson content;

      // acceleration
      content.set("fields/acceleration/arrayValue/values/[0]/doubleValue", String(random(0, 20)));
      content.set("fields/acceleration/arrayValue/values/[1]/doubleValue",  String(random(0, 20)));
      content.set("fields/acceleration/arrayValue/values/[2]/doubleValue", String(random(0, 20)));

      // air_qual
      content.set("fields/air_qual/integerValue", String(random(0, 1200)));

      // speed
      content.set("fields/speed/doubleValue", String(random(0, 100)));

      // temp
      content.set("fields/temp/doubleValue", String(random(25, 35)));

      // device_id
      content.set("fields/device_id/stringValue", DEVICE_ID);

      // disembark
      content.set("fields/disembark/booleanValue", false);

      // embark
      content.set("fields/embark/booleanValue", false);

      // gyroscope
      content.set("fields/gyroscope/arrayValue/values/[0]/doubleValue", String(random(0, 20)));
      content.set("fields/gyroscope/arrayValue/values/[1]/doubleValue",  String(random(0, 20)));
      content.set("fields/gyroscope/arrayValue/values/[2]/doubleValue", String(random(0, 20)));

      // is_active
      content.set("fields/is_active/booleanValue", true);

      // location
      content.set("fields/location/geoPointValue/latitude", coordinates[count][0]);
      content.set("fields/location/geoPointValue/longitude", coordinates[count][1]);

      // passenger_count
      content.set("fields/passenger_count/integerValue", count);

      // route_id
      content.set("fields/route_id/integerValue", 0);

      // Get the current timestamp
      //unsigned long timestamp = now();  // Get the current time
      // Convert timestamp to a string
      String timestampStr = convertToFirebaseTimestamp(timestamp);

      // timestamp
      content.set("fields/timestamp/timestampValue", timestampStr);

      Serial.print("Create a document... ");

      if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
          Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      else
          Serial.println(fbdo.errorReason());

      count++;
    }
}

// ----------------------- HELPER FXNS for WiFi -----------------------
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Waiting for connection...");
  }

  Serial.println("Connected to WiFi!");
}

// ----------------------- HELPER FXNS for Timestamp -----------------------
void configTimeFromNTP() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); // Show the current time
}

void updateTimestamp() {
  if(WiFi.status() == WL_CONNECTED) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    // Use the seconds since the Unix epoch as your timestamp
    timestamp = now;
  } else {
    // If not connected to WiFi, use the ESP32's internal millis() function
    // This won't be accurate over long periods, but will be reasonably accurate in short term
    timestamp += millis() / 1000;
  }
}

String convertToFirebaseTimestamp(unsigned long int unixTime) {
  // Convert UNIX timestamp to ISO 8601 format
  time_t t = unixTime;
  struct tm timeinfo;
  gmtime_r(&t, &timeinfo);
  char timestampStr[22];  // Increased the size to accommodate the 'Z' character
  snprintf(timestampStr, sizeof(timestampStr), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  return String(timestampStr);
}