// TEC
#define CURRENT_SSID "IoT_H3/4"
#define CURRENT_PASSWORD "98806829"
#define CURRENT_API_IP "192.168.0.200" // ??
#define CURRENT_GATEWAY "192.168.0.1" // ??


// MQTT Server TEC
//#define MQTT_SERVER "mosquitto.eclipse"
//#define MQTT_SERVER "192.168.0.130"
//#define MQTT_PORT 1883
//#define MQTT_USER "device02"
//#define MQTT_PASSWORD "device02-password"
//#define MQTT_CLIENT_ID "ESP32Client"
//#define MQTT_TOPIC "sensor/device02/SnifferBoys"

// MQTT Server Egen
//#define MQTT_SERVER "mosquitto.eclipse"
#define MQTT_SERVER "192.168.0.205"
#define MQTT_PORT 8883
#define MQTT_USER "esp32"
#define MQTT_PASSWORD "1234"
#define MQTT_CLIENT_ID "MasterOfLocations"
#define MQTT_TOPIC "esp32/time"

// LittleFS paths
#define SSID_PATH "/ssid.txt"
#define PASSWORD_PATH "/pass.txt"
#define IP_PATH "/ip.txt"
#define GATEWAY_PATH "/gateway.txt"
#define SNIFFER_DATA_PATH "/sniffings.csv"