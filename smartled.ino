on ec2 = ssh -i "C:\Users\sahub\Desktop\myserverkey.pem" ec2-user@3.25.58.199

edit code sudo nano server.js
run code node server.js
kill the run process = sudo kill -9 pid  and sudo kill -9 $(sudo lsof -t -i:8443) # Kill any process using 8443
kill process==== ps aux | grep node
run this for kill== sudo kill -9 7802
currently running server = sudo netstat -tulnp | grep :443
check status of port = curl -k https://3.26.19.121:8443/status
put cirtificates in ec2 = scp -i "C:\Users\sahub\Desktop\myserverkey.pem" "C:\Users\sahub\Desktop\myserverkey.pem" ec2-user@3.27.202.221:/home/ec2-user/certs/
check the file cirt =ls -l /home/ec2-user/certs/

pm2 start and kill process

start=
	sudo npm install -g pm2
	sudo pm2 start server.js
	sudo pm2 save
	sudo pm2 startup
	sudo pm2 list
	sudo pm2 restart myserver

stop server=

	sudo pm2 stop server.js
	sudo pm2 delete server.js

		sudo pm2 stop all
		sudo pm2 delete all



steps 

give permition to key:
icacls "C:\Users\sahub\Desktop\myserverkey.pem" /grant:r "sahub:R"
icacls "C:\Users\sahub\Desktop\myserverkey.pem" /inheritance:r
icacls "C:\Users\sahub\Desktop\myserverkey.pem" /deny "Everyone:F"

nano package.json
{
  "type": "module"
}
sudo yum install -y nodejs npm
sudo yum install -y nodejs
sudo yum update -y

npm install aws-iot-device-sdk
npm install express
npm install cors
npm install

in local cmd
scp -i "C:\Users\sahub\Desktop\myserverkey.pem" "C:\Users\sahub\Desktop\your-private.pem.key" ec2-user@3.25.58.199:/home/ec2-user/certs/
scp -i "C:\Users\sahub\Desktop\myserverkey.pem" "C:\Users\sahub\Desktop\your-certificate.pem.crt" ec2-user@3.25.58.199:/home/ec2-user/certs/
scp -i "C:\Users\sahub\Desktop\myserverkey.pem" "C:\Users\sahub\Desktop\AmazonRootCA1.pem" ec2-user@3.25.58.199:/home/ec2-user/certs/

in ec2 for ssl certificate
mkdir -p /home/ec2-user/certs && cd /home/ec2-user/certs
openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout https-private.pem.key -out https-certificate.pem.crt

check certificate 
ls -l /home/ec2-user/certs/




html running code 
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 LED Control</title>
</head>
<body>
    <h1>ESP32 LED Control</h1>

    <p><strong>Connection Status:</strong> <span id="status">Connecting...</span></p>

    <button onclick="sendCommand('ON')">Turn ON</button>
    <button onclick="sendCommand('OFF')">Turn OFF</button>

    <p id="response"></p>

    <script>
        async function checkConnection() {
            try {
                const response = await fetch("https://3.26.19.121/status"); // ✅ HTTPS URL
                const data = await response.json();
                
                if (data.connected) {
                    document.getElementById("status").innerText = "🟢 Online";
                } else {
                    document.getElementById("status").innerText = "🔴 Offline";
                }
            } catch (error) {
                console.error("❌ Connection Check Failed:", error);
                document.getElementById("status").innerText = "🔴 Offline";
            }
        }
    
        function sendCommand(command) {
            console.log("Sending:", JSON.stringify({ message: command }));
    
            fetch("https://3.26.19.121/send", {  // ✅ HTTPS URL
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ message: command })
            })
            .then(response => response.json())
            .then(data => {
                console.log("✅ Response:", data);
                document.getElementById("response").innerText = "Server Response: " + JSON.stringify(data);
            })
            .catch(error => {
                console.error("❌ Fetch Error:", error);
                document.getElementById("response").innerText = "Error sending command.";
            });
        }
    
        // Check connection status every 5 seconds
        setInterval(checkConnection, 5000);
        checkConnection(); // Initial check when page loads
    </script>
    
</body>
</html>
___________________________________________________________________________________________


server code 

import awsIot from "aws-iot-device-sdk";
import fs from "fs";
import express from "express";
import https from "https";
import cors from "cors";

// AWS IoT Certificate Paths (EC2 Paths)
const keyPath = "/home/ec2-user/certs/your-private.pem.key";
const certPath = "/home/ec2-user/certs/your-certificate.pem.crt";
const caPath = "/home/ec2-user/certs/AmazonRootCA1.pem";

// HTTPS Certificate Paths (for Secure Server)
const httpsKeyPath = "/home/ec2-user/certs/https-private.pem.key";
const httpsCertPath = "/home/ec2-user/certs/https-certificate.pem.crt";

// Verify Certificate Files Exist
[keyPath, certPath, caPath, httpsKeyPath, httpsCertPath].forEach((path) => {
  if (!fs.existsSync(path)) console.error(`❌ File not found: ${path}`);
});

// AWS IoT Device Setup
const device = awsIot.device({
  keyPath,
  certPath,
  caPath,
  clientId: `Bibhu_${Math.floor(Math.random() * 10000)}`, // Unique Client ID
  host: "aebfzuqqcp989-ats.iot.ap-southeast-2.amazonaws.com",
  keepalive: 120,
  reconnectPeriod: 5000,
});

// Track connection status
let isConnected = false;

// Express Server
const app = express();
app.use(express.json());
app.use(cors({ origin: "*" }));

// Root Route - Fixes "Cannot GET /" Issue
app.get("/", (req, res) => {
  res.send("🚀 HTTPS Server is running!");
});

// AWS IoT Event Handlers
device.on("connect", () => {
  console.log("✅ Connected to AWS IoT Core");
  isConnected = true;
  device.subscribe("esp32/control");
});

device.on("close", () => {
  console.log("🔴 Disconnected from AWS IoT Core");
  isConnected = false;
  setTimeout(() => device.reconnect(), 5000);
});

device.on("error", (error) => console.error("⚠️ AWS IoT Error:", error));

device.on("offline", () => console.warn("⚠️ AWS IoT Offline - Connection lost"));

device.on("message", (topic, payload) => {
  console.log(`📩 Message received: ${payload.toString()} on topic ${topic}`);
});

// Status Endpoint
app.get("/status", (req, res) => {
  res.json({ connected: isConnected });
});

// Publish Command to IoT
app.post("/send", (req, res) => {
  const { message } = req.body;
  if (!message) return res.status(400).json({ error: "Message is required" });
  console.log(`📤 Sending to IoT: ${message}`);
  device.publish("esp32/control", JSON.stringify({ message }));
  res.json({ success: true, message: "Command sent" });
});

// Create HTTPS Server
const server = https.createServer({
  key: fs.readFileSync(httpsKeyPath),
  cert: fs.readFileSync(httpsCertPath),
}, app);

server.listen(443, "0.0.0.0", () => {
  console.log("🚀 HTTPS Server Running on https://3.26.19.121");
});
___________________________________________________________________________________________



aurdino code

 #include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "Airtel_Bibhuti sahu";
const char* password = "Bibhutisahu";

// AWS IoT Core MQTT Configuration
const char* aws_mqtt_server = "aaay1ryludbvg-ats.iot.ap-southeast-2.amazonaws.com";
const int aws_mqtt_port = 8883;

const char* subscribeTopic = "esp32/control";
const char* publishTopic = "esp32/status";

WiFiClientSecure espClient;
PubSubClient client(espClient);

const int ledPin = 2;

// Amazon Root CA 1
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUdu/2d227MrUImJBFBOHk4qRftdcwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDMxNTEyMDkz
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANIMb4JPkjSNlMcHgi+r
MGR663dFYyDB5B5oljBITnyQh3kTlhrqOlLTVDuSZiYdD80r+Arof/qsRISnE4E2
6jUUhHyw2Ezj8EmKBahdmSGM3cJpyGMTjXLhxypgsVIcckkKTlPmnBrXE4F2Zfqn
6VTX7/Zsj8qqO+WQ4+3pJV35lnMgrWQOI+bCeMtHBm3IZ9LW7MO+1FS9+/0nHji+
9E4pM1tE3tvAwXyajY4adAV1vu131iH9cHHofPFXppjDuim6o5uAJzfFCJjyzmWD
H76R1FsTTT09Mydh28PNsajkYkjUjGU/vyFhniqwhwT3ma9jkijtXsxUmaHnFNqE
16ECAwEAAaNgMF4wHwYDVR0jBBgwFoAUluvaxJ5RQCT1GjJTPumqYRMTCBIwHQYD
VR0OBBYEFG1wnadto/0sOYYHvbyx45VdKlTMMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCJT714J+mxYMDaeYLm5Ib/fdFn
pIo5kfGWuBCQyzSqfbvn5Omqcx8HGBSkk6JVikb36aT504h5tNvHSmzks2Ln6Qh2
C5kLvZXa04eGotFOiT+qEcOXWindns7YKDrXdRfexc5KfS4ri56VpsI4c9baJLEQ
qkGQoOBoyxQeTtap/Jm4+MAlil/UIPTeJB1niiuEFlSEBqYq4XR3etVIStGqOM9k
Aj+pJgPflGtQx2pza71a9T7e5t9v8agGdmUtzNDHlO1zf/KN4YQmvigXOOoFMx18
0FLMR6hGQJUGIN8RH947WH+7RSQzqXxdZP88Fyqe2klh/Tmp+C6TUgba2/eM
-----END CERTIFICATE-----

)KEY";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA0gxvgk+SNI2UxweCL6swZHrrd0VjIMHkHmiWMEhOfJCHeROW
Guo6UtNUO5JmJh0PzSv4Cuh/+qxEhKcTgTbqNRSEfLDYTOPwSYoFqF2ZIYzdwmnI
YxONcuHHKmCxUhxySQpOU+acGtcTgXZl+qfpVNfv9myPyqo75ZDj7eklXfmWcyCt
ZA4j5sJ4y0cGbchn0tbsw77UVL37/SceOL70TikzW0Te28DBfJqNjhp0BXW+7XfW
If1wceh88VemmMO6Kbqjm4AnN8UImPLOZYMfvpHUWxNNPT0zJ2Hbw82xqORiSNSM
ZT+/IWGeKrCHBPeZr2OSKO1ezFSZoecU2oTXoQIDAQABAoIBAGweZo+G8dUHQ5un
K8XpskEmXGnKV5oW85sLbrNvpKbNpk4WXN+BzX+ws9zuIJJhxy5vAYBr6ORZI/O8
JHgQkkG8S+37AXuioJ7Z4sulUhtI7SUTWkhVWivWN7SCjQOdDN635VWekJ/oMWwk
S53YLGy1shkADIvfUHF8QTbHGarizWHq/0pqa5qRPpsRhaFE4TstR5epiOunCT8h
h0qbUcHj1LJ953bXXoWqN9gN+DOoUuZNi3IbW23txq9qpGwRSeEgwOZPVgSax4RD
oarZnbtyaHl4NxMeTPmZoC/681s9aYX2lthFnj5ktCCA8/yV4iEz1LEqHOxe61s1
4U3ZbIECgYEA6+XAD5X/a85HCm3Jvl4eorK+S5GpkJPgftW5WTv0nQWCeTT+4pbV
hgOLPvdgx0kLUhecYRx8kuI+Ta6jaPf6OxwyfLlutkruNqUJg9L3t/5TPDSvarW3
oPtpaZZltDHSztgPfTPfHInGf4kRiLBEiS55VUsFaa183WnMwT/AzTkCgYEA4/LG
nZyv47lAbVDI5++HRIKgBdx8+9VDoIgan6JrJP3IkZPSC1mdYFDwm0OF9nyTLDRd
VV0eDEC3lO0P6hm9Q3pN/cJMuOHnvgBkyqrdTItw0Ud8ufb9lEwgj9o9XIFt2YkF
zPtXgm5M84SUBDFh2+uVTg4x6sfoJO2d/PFVRakCgYADcdZtI9aX/2ogJ9xDUGKI
I64NsnHUb6RelcZgHDUFOfhexrxWGLfDfjfBUqFyJ4nnx/p7nIvhJTadscgnFz/r
Lw05rweD1/JEkAJkj2cYo7kZbveEymSFEEbIyO5xRCz84kDPpjtC0mw4norIATLv
AeO5sB/DHEpvfUUFKWYqWQKBgQDaD1ddU5UXfhJaANwQtJ1sNqGqucB+kQd+vHTr
nU5uNrD2Qri+zt20U/m4LiIrKAUKVpWqoceq5QY524ZL5o+dy5RsWsFWZBVzN6E0
F7xYC1TDf2Wb6OGwShskQhxNMCvL3D7GVvLh/W5zzrJiSHNbFMThxMaP2OkE2qM2
ZA+08QKBgFs5oD3ZLtLWFuotYFP1w6J6VBGzGEcmRBI5dPmS0T3RiD9hO2H884fv
9LVF+cAagTLCxJaQqx6vmyGd8c953Gwo4ONlsuizdAvnG6oGXngNwonWJzink11F
tx5h6veuDERENTd+zjE34jFtcHWU6WwXer39AB9OmyY3QdjV/0My
-----END RSA PRIVATE KEY-----







)KEY";


void connectToWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }
}

#include <ArduinoJson.h>  // Include the library

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received: ");
    
    // Convert payload to a string
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    // Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("Failed to parse JSON!");
        return;
    }

    // Extract "message" value
    String command = doc["message"];

    // Control LED
    if (command == "ON") {
        digitalWrite(ledPin, HIGH);
        Serial.println("LED turned ON");
    } else if (command == "OFF") {
        digitalWrite(ledPin, LOW);
        Serial.println("LED turned OFF");
    }
}


void connectToAWS() {
    espClient.setCACert(AWS_CERT_CA);
    espClient.setCertificate(AWS_CERT_CRT);
    espClient.setPrivateKey(AWS_CERT_PRIVATE);

    client.setServer(aws_mqtt_server, aws_mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
        Serial.print("Connecting to AWS IoT...");
        if (client.connect("ESP32_Client")) {
            Serial.println("Connected!");
            client.subscribe(subscribeTopic);
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    connectToWiFi();
    connectToAWS();
}

void loop() {
    if (!client.connected()) {
        connectToAWS();
    }
    client.loop();
}
__________________________________________________________________________________________
