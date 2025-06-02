// TEC
//#define CURRENT_SSID "IoT_H3/4"
//#define CURRENT_PASSWORD "98806829"
//#define CURRENT_API_IP "192.168.0.200"
//#define CURRENT_GATEWAY "192.168.0.1"

// HOME
#define CURRENT_SSID "1206"
#define CURRENT_PASSWORD "m43q3swn_2"
#define CURRENT_API_IP "192.168.50.200" // ??
#define CURRENT_GATEWAY "192.168.50.1" // ??

// MQTT Server
//#define MQTT_SERVER "mosquitto.eclipse"
#define MQTT_SERVER "192.168.0.130"
#define MQTT_PORT 1883
#define MQTT_USER "device02"
#define MQTT_PASSWORD "device02-password"
#define MQTT_CLIENT_ID "ESP32Client"
#define MQTT_TOPIC "sensor/device02/SnifferBoys"

// LittleFS paths
#define SSID_PATH "/ssid.txt"
#define PASSWORD_PATH "/pass.txt"
#define IP_PATH "/ip.txt"
#define GATEWAY_PATH "/gateway.txt"
#define SNIFFER_DATA_PATH "/sniffings.csv"