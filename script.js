import awsIot from "aws-iot-device-sdk";
import { WebSocketServer } from "ws";
import fs from "fs";

// Corrected paths for EC2 instance
const keyPath = "/home/ec2-user/certs/your-private.pem.key";
const certPath = "/home/ec2-user/certs/your-certificate.pem.crt";
const caPath = "/home/ec2-user/certs/AmazonRootCA1.pem";

// Check if certificate files exist
if (!fs.existsSync(keyPath)) console.error("❌ Private Key file not found!");
if (!fs.existsSync(certPath)) console.error("❌ Certificate file not found!");
if (!fs.existsSync(caPath)) console.error("❌ CA file not found!");

// AWS IoT Device Configuration
const device = awsIot.device({
    keyPath,
    certPath,
    caPath,
    clientId: "Bibhu",
    host: "aebfzuqqcp989-ats.iot.ap-southeast-2.amazonaws.com"
});

// Handle AWS IoT Connection
device.on("connect", () => {
    console.log("✅ Connected to AWS IoT Core");
    device.subscribe("esp32/control");
});

device.on("message", (topic, payload) => {
    console.log(`📩 Message received: ${payload.toString()} on topic ${topic}`);
});

// WebSocket Server for Web Control
const wss = new WebSocketServer({ port: 8080 });

wss.on("connection", (ws) => {
    console.log("✅ WebSocket Connection Established");

    ws.on("message", (message) => {
        const command = JSON.parse(message).message;
        console.log(`📤 Sending to IoT: ${command}`);
        device.publish("esp32/control", JSON.stringify({ message: command }));
    });

    device.on("message", (topic, payload) => {
        ws.send(payload.toString());
    });
});

console.log("🚀 WebSocket Server Running on ws://3.26.19.121:8080");
