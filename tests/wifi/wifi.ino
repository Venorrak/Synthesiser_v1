#include <WiFi.h>

const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

WiFiServer server(80);
String header;
String body;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (header.indexOf("POST") >= 0) {
              // If it's a POST request, read the body
              while (client.available()) {
                body += client.readStringUntil('\r\n');
              }
            }

            Serial.print("Le body be like ");
            Serial.println(body);

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("Hello World!");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    body = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
