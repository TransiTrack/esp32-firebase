#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Wil"
#define WIFI_PASSWORD "122333444"

/* 2. Define the API Key */
#define API_KEY "AIzaSyDVi1awMUCpMK6P-AejbbG0Ga2jBtD13Vo"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "transitrack-709f1"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "zedec.pacaldo@gmail.com"
#define USER_PASSWORD "password123"

#define DEVICE_ID "jeep0"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

bool taskcomplete = false;


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
  {14.657568, 121.064787}
};

void setup()
{
    Serial.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - dataMillis > 4000 || dataMillis == 0))
    {
        if (count > 23){
          count = 0;
        }

        dataMillis = millis();

        String documentPath = "jeeps/" + String(DEVICE_ID);
        FirebaseJson content;

        // device_id
        content.set("fields/device_id/stringValue", DEVICE_ID);

        // is_embark
        content.set("fields/is_embark/booleanValue", true);

        // passenger_count
        content.set("fields/passenger_count/integerValue", count);

        // route_id
        content.set("fields/route_id/integerValue", 0);

        // timestamp
        content.set("fields/timestamp/timestampValue", "2023-5-09T1:28:10Z"); // RFC3339 UTC "Zulu" format

        // acceleration
        content.set("fields/acceleration/arrayValue/values/[0]/doubleValue", String(random(0, 20)));
        content.set("fields/acceleration/arrayValue/values/[1]/doubleValue",  String(random(0, 20)));
        content.set("fields/acceleration/arrayValue/values/[2]/doubleValue", String(random(0, 20)));

        // location
        content.set("fields/location/geoPointValue/latitude", coordinates[count][0]);
        content.set("fields/location/geoPointValue/longitude", coordinates[count][1]);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "device_id,is_embark,passenger_count,route_id,timestamp,acceleration,location")){
          Serial.printf("Updated Document... count: %d\n%s\n\n", count, fbdo.payload().c_str());
        } else {
          Serial.println(fbdo.errorReason());
        }
        count++;
    }
}
