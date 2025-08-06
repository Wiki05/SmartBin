#define BLYNK_TEMPLATE_ID "TMPL3BklU19mt"
#define BLYNK_TEMPLATE_NAME "SmartBin"
#define BLYNK_AUTH_TOKEN "Ae57Rxk8I_XDopH0cpol6-jKUc14I-c7"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const char* iftttWebhookURL = "https://maker.ifttt.com/trigger/bin_full/json/with/key/f1PnzL1V-VmmBjFUKuAGD09fC2OyvyQ_GB9I2S4eJ3H";

#define TRIG_PIN     5
#define ECHO_PIN     18
#define LED_PIN      23

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;
bool emailSent = false;

void sendEmailNotification() {
  if (!emailSent) {
    HTTPClient http;
    http.begin(iftttWebhookURL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"value1\":\"SmartBin Alert\",\"value2\":\"Bin is FULL\",\"value3\":\"Distance below threshold\"}";
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.println("[IFTTT] Email sent successfully.");
    } else {
      Serial.print("[IFTTT] Email failed. Code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    emailSent = true;
  }
}

void checkBinStatus() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  Serial.print("[Sensor] Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm   ");

  Blynk.virtualWrite(V0, distance); 

  if (distance > 0 && distance <= 10) {
    digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Bin is FULL!     ");
    Serial.println("[ALERT] Bin is FULL - LED ON");

    Blynk.logEvent("bin_full");
    sendEmailNotification();
  } else {
    digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Status: CLEAN    ");
    Serial.println("[STATUS] Bin is clean - LED OFF");
    emailSent = false;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, pass);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected.");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected  ");
  } else {
    Serial.println("\nWiFi Failed!");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed     ");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(3000L, checkBinStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}
