
/*
 * DIGITAL DINGDONG CONTROLLER (10 BUTTONS) - ARDUINO NANO
 *
 * Protocol:
 * 10 ASCII STATUS + '*' + CRC-8 CHECKSUM (2 HEX) + ENTER (CR+LF)
 *
 * Contoh: 0000000000*C0\r\n
 * Byte 1..10 : Status 10 tombol sesuai buttonPins
 * Byte 11    : Separator '*'
 * Byte 12..13: Checksum CRC-8 (2 digit HEX)
 * Byte 14..15: Enter (\r\n)
 *
 * Baudrate     : 115200
 * Debounce     : 20 ms
 * Send rate    : 10 ms (saat aktif)
 * Release hold : 3000 ms
 * Idle rate    : 500 ms (saat tidak ada aktivitas)
 *
 * Button status:
 * 1 = Pressed
 * 0 = Released
 */

#define BUTTON_COUNT 10

// =====================================
// MODE PENGIRIMAN
// =====================================
// 1 = MODE_SERIAL_RAW    (Format: 10 status + '*' + CRC-8 + interval 10ms + heartbeat 500ms)
// 2 = MODE_KEYBOARD_MODE (Format: Hanya kirim saat tombol ditekan/dilepas untuk bridge Keyboard)
byte currentMode = 1;

#define DEBOUNCE_MS 20
#define SEND_INTERVAL_MS 10
#define RELEASE_HOLD_MS 3000
#define IDLE_INTERVAL_MS 500 // Interval pengiriman saat menganggur (500 ms / 0.5 detik)

// Urutan tombol
const byte buttonPins[BUTTON_COUNT] = {A4, A3, A2, A1, A0, 9, 8, 7, 6, 5};

// Status tombol
bool buttonState[BUTTON_COUNT];

// Pembacaan mentah
bool lastRawState[BUTTON_COUNT];

// Waktu debounce masing-masing tombol
unsigned long lastDebounceTime[BUTTON_COUNT];

// Timer komunikasi
unsigned long lastSendTime = 0;
unsigned long lastActivityTime = 0;

// Payload 10 karakter
char payload[BUTTON_COUNT + 1];

// Status apakah ada tombol ditekan
bool anyButtonPressed = false;

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  for (byte i = 0; i < BUTTON_COUNT; i++) {

    pinMode(buttonPins[i], INPUT_PULLUP);

    bool reading = digitalRead(buttonPins[i]);

    lastRawState[i] = reading;

    // LOW = pressed
    buttonState[i] = (reading == LOW);

    lastDebounceTime[i] = millis();
  }

  updatePayload();

  lastActivityTime = millis();

  // Kirim status awal
  sendPacket();
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  unsigned long now = millis();

  bool stateChanged = false;

  // Baca seluruh tombol
  for (byte i = 0; i < BUTTON_COUNT; i++) {

    bool reading = digitalRead(buttonPins[i]);

    // Deteksi perubahan pembacaan mentah
    if (reading != lastRawState[i]) {

      lastRawState[i] = reading;

      lastDebounceTime[i] = now;
    }

    // Debounce
    if ((now - lastDebounceTime[i]) >= DEBOUNCE_MS) {

      bool newState = (reading == LOW);

      if (newState != buttonState[i]) {

        buttonState[i] = newState;

        stateChanged = true;
      }
    }
  }

  // =====================================
  // PENGIRIMAN DATA
  // =====================================

  // 1. Kirim seketika saat ada perubahan tombol (Berlaku untuk Mode 1 & 2)
  if (stateChanged) {

    updatePayload();

    lastActivityTime = now;

    sendPacket();
  }

  // 2. Pengiriman periodik (HANYA AKTIF DI MODE 1: SERIAL RAW)
  // Pada Mode 2 (Keyboard Mode), pengiriman periodik dimatikan agar tidak spamming/flooding
  if (currentMode == 1) {

    anyButtonPressed = false;

    for (byte i = 0; i < BUTTON_COUNT; i++) {

      if (buttonState[i]) {

        anyButtonPressed = true;

        break;
      }
    }

    unsigned long currentInterval;

    if (anyButtonPressed || (now - lastActivityTime < RELEASE_HOLD_MS)) {
      currentInterval = SEND_INTERVAL_MS; // Cepat (10 ms)
    } else {
      currentInterval = IDLE_INTERVAL_MS; // Standby (1 detik)
    }

    if (now - lastSendTime >= currentInterval) {
      sendPacket();
    }
  }
}

// =====================================
// UPDATE PAYLOAD
// =====================================

void updatePayload() {

  for (byte i = 0; i < BUTTON_COUNT; i++) {

    payload[i] = buttonState[i] ? '1' : '0';
  }

  payload[BUTTON_COUNT] = '\0';
}

// =====================================
// CRC-8 CHECKSUM
// =====================================

byte calculateChecksum() {

  byte crc = 0x00;

  for (byte i = 0; i < BUTTON_COUNT; i++) {

    crc ^= (byte)payload[i];

    for (byte b = 0; b < 8; b++) {

      if (crc & 0x80) {

        crc = (crc << 1) ^ 0x07;

      } else {

        crc <<= 1;
      }
    }
  }

  return crc;
}

// =====================================
// SEND PACKET
// =====================================

void sendPacket() {

  byte checksum = calculateChecksum();

  const char hexDigits[] = "0123456789ABCDEF";

  char hexHigh = hexDigits[(checksum >> 4) & 0x0F];
  char hexLow  = hexDigits[checksum & 0x0F];

  // 1. 10 status tombol (mulai dari byte pertama sesuai urutan buttonPins)
  Serial.print(payload);

  // 2. Separator '*'
  Serial.print('*');

  // 3. Checksum (2 digit HEX)
  Serial.print(hexHigh);
  Serial.print(hexLow);

  // 4. Akhiran Enter (CR + LF)
  Serial.println();

  lastSendTime = millis();
}
