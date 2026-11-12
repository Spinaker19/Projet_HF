#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

// Define the Wi-Fi channels to cycle through
const int channels[] = {1, 6, 11}; 
const int numChannels = sizeof(channels) / sizeof(channels[0]);

// List of SSIDs (Network Names) you want to broadcast
const char* ssids[] = {
  "🌊 Massive Wave 1",
  "🌊 Massive Wave 2",
  "🌊 Massive Wave 3",
  "☠️ Error: 404 Found",
  "📡 FBI Surveillance Van"
};
const int numSsids = sizeof(ssids) / sizeof(ssids[0]);

// Base template for a raw 802.11 Beacon Frame
uint8_t packet[128] = {
  0x80, 0x00,                         // Type/Subtype: Beacon Frame
  0x00, 0x00,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (Will be randomized)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID (Will be randomized)
  0x00, 0x00,                         // Sequence number
  // Fixed Parameters (Timestamp, Beacon Interval, Capabilities)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00, 
  0x31, 0x04
};

void setup() {
  Serial.begin(115200);
  
  // Initialize Wi-Fi in Station Mode
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
}

void loop() {
  for (int c = 0; c < numChannels; c++) {
    // Switch channels to spread the broadcast across the spectrum
    esp_wifi_set_channel(channels[c], WIFI_SECOND_CHAN_NONE);
    
    for (int i = 0; i < numSsids; i++) {
      int ssidLen = strlen(ssids[i]);
      
      // Randomize the Source MAC and BSSID so they appear as unique devices
      for (int m = 10; m < 22; m++) {
        packet[m] = random(256);
      }
      
      // Construct the Tagged Parameters (SSID)
      packet[36] = 0x00;        // Tag Number: SSID
      packet[37] = ssidLen;     // Tag Length
      memcpy(&packet[38], ssids[i], ssidLen);
      
      // Supported Rates Tag
      int pos = 38 + ssidLen;
      packet[pos++] = 0x01; packet[pos++] = 0x08;
      packet[pos++] = 0x82; packet[pos++] = 0x84;
      packet[pos++] = 0x8b; packet[pos++] = 0x96;
      packet[pos++] = 0x24; packet[pos++] = 0x30;
      packet[pos++] = 0x48; packet[pos++] = 0x6c;
      
      // DS Parameter Set (Current Channel)
      packet[pos++] = 0x03; packet[pos++] = 0x01;
      packet[pos++] = channels[c];
      
      // Send the raw packet into the air
      esp_wifi_80211_tx(WIFI_IF_STA, packet, pos, true);
      delay(5); // Small delay to prevent crashing the radio buffer
    }
  }
}
