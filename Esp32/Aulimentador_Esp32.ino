#include "BluetoothSerial.h"
#include <WiFi.h>
#include <Preferences.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

#define LED_BUILTIN 2
#define SERVO_PIN 5

BluetoothSerial SerialBT;
Preferences preferences;
Servo servo;

WiFiClient espClient;
PubSubClient client(espClient);

// ===== Variáveis =====

String ssid;
String password;

float gramasPorSegundo = 29.0;
float gramasNoPote = 0;
float alvo = 0;

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 10000;

String ultimoHorarioRecebido = "00:00";

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);
  delay(500);

  pinMode(LED_BUILTIN, OUTPUT);

  servo.attach(SERVO_PIN);

  Serial.println("ESP32 funcionando");

  SerialBT.begin("ESP32-Alimentador");

  preferences.begin("configwifi", false);

  carregarWiFi();

  conectarWiFi();

  client.setServer("test.mosquitto.org", 1883);
  client.setCallback(mqttCallback);

  ultimoHorarioRecebido = preferences.getString("ultimoHorario", "00:00");
}

// =================================================
// LOOP
// =================================================

void loop() {

  garantirMQTT();

  client.loop();

  indicadorWiFi();

  receberWiFiBluetooth();

  publicarPesoPeriodico();

  processarDispensador();
}

// =================================================
// WIFI
// =================================================

void carregarWiFi() {

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
}

void conectarWiFi() {

  if (ssid.length() == 0) return;

  Serial.print("Conectando ao WiFi");

  WiFi.begin(ssid.c_str(), password.c_str());

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {

    Serial.print(".");
    delay(500);
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\nWiFi conectado!");

  } else {

    Serial.println("\nFalha ao conectar WiFi");
  }
}

void indicadorWiFi() {

  if (WiFi.status() == WL_CONNECTED) {

    digitalWrite(LED_BUILTIN, HIGH);

  } else {

    digitalWrite(LED_BUILTIN, LOW);
  }
}

// =================================================
// MQTT
// =================================================

void garantirMQTT() {

  if (client.connected()) return;

  reconnect();
}

void reconnect() {

  while (!client.connected()) {

    if (client.connect("ESP32Alimentador")) {

      client.subscribe("Esp32/GramasEnvio");
      client.subscribe("Esp32/HorarioEnvio");

      client.publish("Esp32/PesoAtual", String(gramasNoPote).c_str(), true);
      client.publish("Esp32/UltimoPeso", String(gramasNoPote).c_str(), true);
      client.publish("Esp32/UltimoHorario", ultimoHorarioRecebido.c_str(), true);

    } else {

      delay(1000);
    }
  }
}

// =================================================
// MQTT CALLBACK
// =================================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String mensagem = String((char*)payload, length);

  if (String(topic) == "Esp32/GramasEnvio") {

    alvo = mensagem.toFloat();

    Serial.println("Solicitado: " + String(alvo) + " g");
  }

  if (String(topic) == "Esp32/HorarioEnvio") {

    ultimoHorarioRecebido = mensagem;

    Serial.println("Horário recebido: " + ultimoHorarioRecebido);

    preferences.putString("ultimoHorario", ultimoHorarioRecebido);
  }
}

// =================================================
// BLUETOOTH WIFI
// =================================================

void receberWiFiBluetooth() {

  if (!SerialBT.available()) return;

  String sinal = SerialBT.readStringUntil('\n');

  int separador = sinal.indexOf(';');

  if (separador <= 0) return;

  ssid = sinal.substring(0, separador);
  password = sinal.substring(separador + 1);

  salvarWiFi(ssid, password);

  WiFi.begin(ssid.c_str(), password.c_str());
}

void salvarWiFi(String s, String p) {

  preferences.putString("ssid", s);
  preferences.putString("password", p);

  Serial.println("Dados salvos no NVS");
}

// =================================================
// PUBLICAÇÃO PERIODICA
// =================================================

void publicarPesoPeriodico() {

  if (millis() - ultimoEnvio < INTERVALO_ENVIO) return;

  client.publish("Esp32/PesoAtual", String(gramasNoPote).c_str(), true);

  ultimoEnvio = millis();
}

// =================================================
// DISPENSADOR
// =================================================

void processarDispensador() {

  if (alvo <= 0) return;

  float tempoTotalSeg = alvo / gramasPorSegundo;

  int tempoMs = tempoTotalSeg * 1000;

  Serial.println("Dispensando por " + String(tempoTotalSeg, 3) + " segundos");

  client.publish("Esp32/Status", "Dispensando", true);

  servo.write(45);

  delay(tempoMs);

  servo.write(0);

  gramasNoPote = alvo;

  client.publish("Esp32/PesoAtual", String(gramasNoPote).c_str(), true);
  client.publish("Esp32/UltimoPeso", String(gramasNoPote).c_str(), true);
  client.publish("Esp32/UltimoHorario", ultimoHorarioRecebido.c_str(), true);

  client.publish("Esp32/Status", "Finalizado", true);

  alvo = 0;
}