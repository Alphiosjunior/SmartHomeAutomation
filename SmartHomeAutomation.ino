#include <DHT.h>
#include <DHT_U.h>
#include "thingProperties.h"

// ── Pin Definitions ──────────────────────────────────────────────────────────
#define LED            8
#define BUTTON         7
#define PIR_BEDROOM    3
#define PIR_LOUNGE     4
#define BEDROOM_LED    9
#define LOUNGE_LED     10
#define FLAME_SENSOR   2
#define DHTPIN         5
#define DHTTYPE        DHT11
#define WATER_PUMP     11
#define FAN            6

// ── State Variables ───────────────────────────────────────────────────────────
bool flameDetected   = false;
bool ledState        = false;
bool lastButtonState = HIGH;
bool autoFanControl  = true;

DHT dht(DHTPIN, DHTTYPE);

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  delay(1500);

  // Initialise Arduino Cloud properties (defined in thingProperties.h)
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Configure pins
  pinMode(LED,           OUTPUT);
  pinMode(BEDROOM_LED,   OUTPUT);
  pinMode(LOUNGE_LED,    OUTPUT);
  pinMode(BUTTON,        INPUT_PULLUP);
  pinMode(PIR_BEDROOM,   INPUT);
  pinMode(PIR_LOUNGE,    INPUT);
  pinMode(FLAME_SENSOR,  INPUT);
  pinMode(WATER_PUMP,    OUTPUT);
  pinMode(FAN,           OUTPUT);

  digitalWrite(LED, ledState);

  dht.begin();
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

// ── Main Loop ─────────────────────────────────────────────────────────────────
void loop() {
  // Sync with Arduino Cloud
  ArduinoCloud.update();

  // ── Read DHT11 sensor ──
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  if (!isnan(temperature)) {
    tempareture = temperature;          // update cloud variable
    Serial.print("Temperature: ");
    Serial.println(tempareture);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // ── Button: arm / disarm security ──
  bool currentButtonState = digitalRead(BUTTON);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    buttonPressed = !buttonPressed;
    systemArmed   = !systemArmed;
  }
  lastButtonState = currentButtonState;

  // ── Read PIR & flame sensors ──
  motionBedroom = digitalRead(PIR_BEDROOM);
  motionLounge  = digitalRead(PIR_LOUNGE);
  flameDetected = digitalRead(FLAME_SENSOR);
  fIRE          = flameDetected;

  // ── Fire suppression ──
  digitalWrite(WATER_PUMP, flameDetected ? HIGH : LOW);

  // ── Security / alert LED ──
  if (flameDetected || (systemArmed && (motionBedroom || motionLounge))) {
    ledState = true;
  } else {
    ledState = false;
  }
  iNTRUDER = ledState;
  digitalWrite(LED, ledState);

  // ── Room lighting (disarmed mode only) ──
  if (!systemArmed) {
    if (!bedroom)    digitalWrite(BEDROOM_LED, motionBedroom ? HIGH : LOW);
    if (!livingroom) digitalWrite(LOUNGE_LED,  motionLounge  ? HIGH : LOW);
  } else {
    digitalWrite(BEDROOM_LED, LOW);
    digitalWrite(LOUNGE_LED,  LOW);
  }

  // ── Automatic fan control ──
  if (autoFanControl) {
    if (temperature > 30.0) {
      digitalWrite(FAN, HIGH);
      Serial.println("Fan ON (Auto)");
    } else {
      digitalWrite(FAN, LOW);
      Serial.println("Fan OFF (Auto)");
    }
  }

  delay(50);
}

// ── Cloud Callbacks ───────────────────────────────────────────────────────────

// Manual fan / climate control from cloud
void onClimateControlChange() {
  autoFanControl = !climateControl;   // disable auto if manual is on
  digitalWrite(FAN, climateControl ? HIGH : LOW);
  Serial.println(climateControl ? "Climate control ON, Fan ON (Manual)"
                                : "Climate control OFF, Fan OFF (Manual)");
}

// Bedroom light (cloud toggle)
void onBedroomChange() {
  digitalWrite(BEDROOM_LED, bedroom ? HIGH : LOW);
}

// Living-room light (cloud toggle)
void onLivingroomChange() {
  digitalWrite(LOUNGE_LED, livingroom ? HIGH : LOW);
}

// Arm / disarm from cloud
void onSercurityArmChange() {
  systemArmed = sercurityArm;
}

// Temperature map (reserved for future use)
void onMapTempChange() {
  // Extend here to map temperature ranges to actions
}
