#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

const char* ssid = "Dev";
const char* password = "868686868";

const char* mqtt_server = "44999388b4dc4398947b13c72f75e7b1.s2.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "Esp8266Demo";
const char* mqtt_password = "Esp8266Demo";

const char* mqtt_topic_RGB = "RGB";
const char* mqtt_topic_CTNgu = "CTNgu";
const char* mqtt_topic_CTBep = "CTBep";
const char* mqtt_topic_Gas = "Gas";
const char* mqtt_topic_Lua = "Lua";
const char* mqtt_topic_Mua = "Mua";
const char* mqtt_topic_CTam = "Tam";
const char* mqtt_topic_Pir = "Pir";

// LED pins
const int touchBep = 2; // D4
const int touchNgu = 14; // D5
const int redPin = 15;// D8
const int greenPin = 13;//D7
const int bluePin = 12;//D6
const int ledBep = 0; // D2
const int quatNgu = 16;// D0


int redValue = 255;
int greenValue = 0;
int blueValue = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

SoftwareSerial mySerial(4, 5); // RX, TX (change pins as needed)

bool isSwitchOn = false;
bool setColor = false;
bool isSwitchOn1 = false;

// Biến để theo dõi trạng thái trước đó của công tắc
bool prevTouchNguState = LOW;
bool prevTouchBepState = LOW;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  // Convert message to string
  String messageStr;
  for (int i = 0; i < length; i++) {
    messageStr += (char)message[i];
  }
  Serial.println(messageStr);
  // Xử lý tín hiệu màu sắc từ MQTT
  if (messageStr.length() == 6) {
    String redHex = messageStr.substring(0, 2);
    String greenHex = messageStr.substring(2, 4);
    String blueHex = messageStr.substring(4, 6);
    redValue = strtol(redHex.c_str(), NULL, 16);
    greenValue = strtol(greenHex.c_str(), NULL, 16);
    blueValue = strtol(blueHex.c_str(), NULL, 16);
    setColor = true;
  }
  if (String(topic) == mqtt_topic_CTNgu) {
    // Xử lý tín hiệu từ MQTT để điều khiển công tắc
    if (messageStr == "ON") {
      setColor = true;
      isSwitchOn = true;
    } else if (messageStr == "OFF") {
      isSwitchOn = false;
    }
  }
  if (String(topic) == mqtt_topic_CTBep) {
    if (messageStr == "ON") {
      isSwitchOn1 = true;
    } else if (messageStr == "OFF") {
      isSwitchOn1 = false;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_CTNgu);
      client.subscribe(mqtt_topic_CTBep);
      client.subscribe(mqtt_topic_RGB);
      // Publish the current switch state on MQTT when connected, but only if it has changed
      if (isSwitchOn) {
        client.publish(mqtt_topic_CTNgu, "ON");
      } else {
        client.publish(mqtt_topic_CTNgu, "OFF");
      }
      if (isSwitchOn1) {
        client.publish(mqtt_topic_CTBep, "ON");
      } else {
        client.publish(mqtt_topic_CTBep, "OFF");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(ledBep, OUTPUT);
  pinMode(touchNgu, INPUT);
  pinMode(touchBep, INPUT);
  pinMode(quatNgu, OUTPUT);
  Serial.begin(115200);
  mySerial.begin(115200);
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
}

void loop() {
   if (mySerial.available()) {
    String receivedData = mySerial.readStringUntil('\n'); // Đọc đến ký tự xuống dòng
    receivedData.trim(); // Xóa ký tự trắng thừa từ đầu và cuối
    // Serial.println(receivedData);
    // Phân tích dữ liệu thành từng giá trị
    String values[5]; // Mảng chứa các giá trị
    int valueIndex = 0;
    int startIndex = 0;
    int commaIndex = receivedData.indexOf(',');

    while (commaIndex != -1) {
      values[valueIndex] = receivedData.substring(startIndex, commaIndex);
      startIndex = commaIndex + 1;
      commaIndex = receivedData.indexOf(',', startIndex);
      valueIndex++;
    }

    values[valueIndex] = receivedData.substring(startIndex); // Giá trị cuối cùng

    // In từng giá trị ra Serial Monitor
    for (int i = 0; i < 5; i++) {
      Serial.print("Value ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(values[i].toInt());
        // Gửi giá trị lên MQTT theo topic tương ứng
      if (i == 0) {
        client.publish(mqtt_topic_Gas, values[i].c_str());
      } else if (i == 1) {
        client.publish(mqtt_topic_Lua, values[i].c_str());
      } else if (i == 2) {
        client.publish(mqtt_topic_CTam, values[i].c_str());
      } else if (i == 3) {
        client.publish(mqtt_topic_Pir, values[i].c_str());
      } else if (i == 4) {
        client.publish(mqtt_topic_Mua, values[i].c_str());
      }
    }
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int touchNguState = digitalRead(touchNgu);
  int touchBepState = digitalRead(touchBep);
  // Serial.println(touchBepState);
  // Xử lý chạm công tắc và gửi dữ liệu lên MQTT nếu trạng thái thay đổi
  if (touchNguState != prevTouchNguState) {
    if (touchNguState == HIGH) {
      client.publish(mqtt_topic_CTNgu, "ON");
      isSwitchOn = true;
      setColor = true;
    } else {
      client.publish(mqtt_topic_CTNgu, "OFF");
      isSwitchOn = false;
      setColor = false;
    }
    prevTouchNguState = touchNguState;
  }

  if (touchBepState != prevTouchBepState) {
    if (touchBepState == HIGH) {
      client.publish(mqtt_topic_CTBep, "ON");
      isSwitchOn1 = true;
    } else {
      client.publish(mqtt_topic_CTBep, "OFF");
      isSwitchOn1 = false;
    }
    prevTouchBepState = touchBepState;
  }
  if(isSwitchOn1) {
    digitalWrite(ledBep, HIGH);
  } else {
    digitalWrite(ledBep, LOW);
  }
  if(isSwitchOn) {
    digitalWrite(quatNgu , HIGH);
  } else {
     digitalWrite(quatNgu , LOW);
  }
  // Xử lý màu sắc
  if (isSwitchOn && setColor) {
    analogWrite(redPin, redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin, blueValue);
  } else {
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
  }
}
