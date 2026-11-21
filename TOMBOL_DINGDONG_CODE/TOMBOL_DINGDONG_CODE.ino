
/*
 * ARDUINO NANO - 10 BUTTON CONTROLLER
 *
 * Protocol:
 * 10 ASCII STATUS (urutan sesuai buttonPins) + ENTER (CR+LF)
 *
 * Contoh: 0000000000\r\n
 * Byte 1  : buttonPins[0] (A0)
 * Byte 2  : buttonPins[1] (A1)
 * ...
 * Byte 10 : buttonPins[9] (5)
 * Akhiran : Enter (\r\n)
 *
 * Baudrate     : 115200
 * Debounce     : 20 ms
 * Send rate    : 10 ms (saat aktif)
 * Release hold : 3000 ms
 * Idle rate    : 1000 ms (saat tidak ada aktivitas)
 *
 * Button status:
 * 1 = Pressed
 * 0 = Released
 */

#define BUTTON_COUNT 10

#define DEBOUNCE_MS 20
#define SEND_INTERVAL_MS 10
#define RELEASE_HOLD_MS 3000
#define IDLE_INTERVAL_MS 1000 // Interval pengiriman saat menganggur (1 detik)

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

  // Ada perubahan status tombol
  if (stateChanged) {

    updatePayload();

    lastActivityTime = now;

    // Kirim langsung
    sendPacket();
  }

  // Periksa apakah ada tombol aktif
  anyButtonPressed = false;

  for (byte i = 0; i < BUTTON_COUNT; i++) {

    if (buttonState[i]) {

      anyButtonPressed = true;

      break;
    }
  }

  // Pengiriman periodik (Interval dinamis menggunakan millis, tanpa delay)
  unsigned long currentInterval;

  if (anyButtonPressed || (now - lastActivityTime < RELEASE_HOLD_MS)) {
    currentInterval = SEND_INTERVAL_MS; // Mode cepat (10 ms) saat tombol
                                        // ditekan atau baru dilepas
  } else {
    currentInterval = IDLE_INTERVAL_MS; // Mode lambat/standby (1000 ms / 1
                                        // detik) saat tidak ada aktivitas
  }

  if (now - lastSendTime >= currentInterval) {
    sendPacket();
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
// SEND PACKET
// =====================================

void sendPacket() {

  // Kirim langsung 10 byte status sesuai urutan buttonPins mulai dari byte
  // pertama + Enter (CRLF)
  Serial.println(payload);

  lastSendTime = millis();
}
