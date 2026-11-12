#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

// Trame Beacon brute pré-formatée (Canal 5 - 2432 MHz)
// Ce paquet contient une fausse trame balise Wi-Fi facilement identifiable.
uint8_t packet[] = {
  0x80, 0x00,                   // Type : Management Frame (Beacon)
  0x00, 0x00,                   // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination : Broadcast
  0x22, 0x33, 0x44, 0x55, 0x66, 0x77, // Source MAC (Fictif)
  0x22, 0x33, 0x44, 0x55, 0x66, 0x77, // BSSID
  0x00, 0x00,                   // Sequence number
  // --- Charge utile fixe (Wireless Management) ---
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
  0x64, 0x00,                   // Beacon Interval (100 TU)
  0x11, 0x00,                   // Capabilities
  // Tag: SSID (SDR_TEST)
  0x00, 0x08, 'S', 'D', 'R', '_', 'T', 'E', 'S', 'T'
};

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Démarrage de l'injecteur SDR pour ESP32-C3 ---");

  // 1. Initialisation en mode Station (requis pour l'injection brute)
  WiFi.mode(WIFI_STA);
  
  // 2. Configuration stricte de l'ESP32-C3 (Ordre obligatoire imposé par l'ESP-IDF)
  esp_wifi_stop(); 

  // Forcer le mode 802.11b (Génère une cloche DSSS large de 22 MHz, très visible au SDR)
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B);

  // CRITIQUE ESP32-C3 : Fixer le débit à 1 Mbps AVANT le esp_wifi_start()
  esp_err_t err_rate = esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_1M_L);
  if (err_rate == ESP_OK) {
    Serial.println("[OK] Débit radio configuré à 1 Mbps (Signal large et lent).");
  } else {
    Serial.printf("[ERREUR] Impossible de régler le débit : 0x%X\n", err_rate);
  }

  // Réduire légèrement la puissance d'émission pour éviter les reboots (Brownouts) sur le C3
  esp_wifi_set_max_tx_power(80); // ~10 dBm au lieu de 20 dBm maximum

  // 3. Démarrage de la radio avec les paramètres appliqués
  esp_wifi_start();

  // 4. Fixer le canal matériel sur le Canal 5 (2432 MHz)
  esp_wifi_set_channel(5, WIFI_SECOND_CHAN_NONE);
  Serial.println("[OK] Radio démarrée sur le Canal 1 (2432 MHz). Injection en cours...");
}

void loop() {
  // Envoi forcé d'une trame brute toutes les 5 millisecondes.
  // Cette cadence élevée crée une ligne continue et ultra-brillante sur le waterfall du SDR.
  esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), true);
  // delay(5); 
}
