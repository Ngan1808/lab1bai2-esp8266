#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "UiTiOt-E3.1";          // Tên mạng Wi-Fi
const char* password = "UiTiOtAP";  // Mật khẩu mạng Wi-Fi
const char* mqtt_server = "broker.mqtt-dashboard.com"; // Địa chỉ IP của MQTT broker
const int mqtt_port = 1883;                 // Cổng MQTT broker

//const char* mqtt_username = "nhom6";
//const char* mqtt_password = "123456789";

char client_id[30];  // ID của ESP8266

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
const long interval = 10000;  // Gửi dữ liệu lên định kỳ mỗi 10 giây

// Chủ đề MQTT chung để gửi cấu hình
const char* controlTopic = "my-device/control";
const char* statusTopic = "my-device/status";

void setup() {
  Serial.begin(115200);
  delay(10);

  // Khởi động kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Khởi động kết nối MQTT
  client.setServer(mqtt_server, mqtt_port);
  sprintf(client_id, "esp8266-%d", ESP.getChipId());
  client.setCallback(callback);

  // Kết nối đến MQTT broker
  if (client.connect(client_id)) {
    Serial.println("Connected to MQTT broker");
    // Theo dõi cấu hình cho thiết bị này
    client.subscribe(statusTopic);
    client.subscribe(controlTopic); 
  } else {
    Serial.println("Failed to connect to MQTT broker, rc=");
    Serial.print(client.state());
    Serial.println(" Try again in 5 seconds");
    delay(5000);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // Gửi dữ liệu lên định kỳ
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Gọi hàm gửi dữ liệu ở đây
    sendData();
  }

  // Đợi 1 giây trước khi thực hiện vòng lặp tiếp theo
  delay(1000);

  // Lắng nghe và xử lý các tin nhắn MQTT
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Xử lý tin nhắn MQTT khi nhận được
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Đọc dữ liệu từ payload
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Payload: ");
  Serial.println(message);
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    client.publish(statusTopic, "on");
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    client.publish(statusTopic, "off");
  }

}

void sendData() {
    
   // Gửi cấu hình cho toàn bộ thiết bị qua chủ đề MQTT chung
  String globalMessage = "Global configuration for all devices";
  client.publish(controlTopic, globalMessage.c_str());

  // Gửi cấu hình cho thiết bị cụ thể qua chủ đề MQTT của nó
  String deviceMessage = "New configuration for device1";
  client.publish(statusTopic, deviceMessage.c_str());

  // Khởi tạo biến topic và dataToSend ở đây
  String topic = "data/" + String(client_id); 
  String dataToSend = "Hello, World!";

  if (client.publish(topic.c_str(), dataToSend.c_str())) {
    Serial.println("Data sent successfully.");
  } else {
    Serial.println("Failed to send data.");
  }
}

void reconnect() {
  // Kết nối lại đến MQTT broker
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(client_id)) {
      Serial.println("Connected to MQTT broker");
      // Subscribe vào chủ đề để nhận lệnh điều khiển
      client.subscribe(statusTopic);
     // Publish trạng thái ban đầu (ví dụ: "off")
      client.publish(statusTopic, "off","on");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }    
  }
}
