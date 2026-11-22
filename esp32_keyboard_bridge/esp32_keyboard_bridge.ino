/*
 * =========================================================================
 * ESP32 KEYBOARD BRIDGE (UART -> HID KEYBOARD)
 * Menerima serial dari Arduino Nano dan bertindak sebagai Keyboard Komputer
 * =========================================================================
 *
 * Mendukung 2 mode output:
 * 1. BLUETOOTH BLE KEYBOARD (ESP32 WROOM / ESP32-S3)
 *    Library: "ESP32-BLE-Keyboard" by T-vK (Install via Library Manager)
 * 2. USB NATIVE HID KEYBOARD (ESP32-S2 / ESP32-S3)
 *    Menggunakan USB bawaan ESP32 board package
 *
 * Wiring dari Arduino Nano ke ESP32:
 * - Arduino Nano TX (D1)  --->  ESP32 RX (GPIO 16)
 * - Arduino Nano GND      --->  ESP32 GND
 * - Arduino Nano 5V       --->  ESP32 VIN / 5V
 */

// =========================================================================
// PILIH METODE KEYBOARD (Pilih salah satu)
// =========================================================================
#define MODE_BLUETOOTH   // Aktifkan ini untuk ESP32 biasa (Wireless Bluetooth)
// #define MODE_USB_NATIVE // Aktifkan ini jika menggunakan ESP32-S2 / ESP32-S3 (Kabel USB)

#if defined(MODE_BLUETOOTH)
  #include <BleKeyboard.h>
  BleKeyboard keyboard("Dingdong Arcade Controller", "DorisJuarsa", 100);
#elif defined(MODE_USB_NATIVE)
  #include "USB.h"
  #include "USBHIDKeyboard.h"
  USBHIDKeyboard keyboard;
#endif

// Pin Serial Hardware 2 di ESP32 untuk menerima data dari Arduino Nano
#define RXD2 16
#define TXD2 17

#define BUTTON_COUNT 10

// =========================================================================
// MAPPING 10 TOMBOL DINGDONG KE TOMBOL KEYBOARD PC
// Sesuaikan huruf/tombol sesuai konfigurasi emulator / game Anda
// =========================================================================
const uint8_t keyMap[BUTTON_COUNT] = {
  'a',  // Tombol 1 (A4 di Nano)
  's',  // Tombol 2 (A3 di Nano)
  'd',  // Tombol 3 (A2 di Nano)
  'f',  // Tombol 4 (A1 di Nano)
  'g',  // Tombol 5 (A0 di Nano)
  'h',  // Tombol 6 (D9 di Nano)
  'j',  // Tombol 7 (D8 di Nano)
  'k',  // Tombol 8 (D7 di Nano)
  'l',  // Tombol 9 (D6 di Nano)
  ' '   // Tombol 10 (D5 di Nano) -> Contoh: Spasi (Spacebar)
};

// Penyimpan status tombol sebelumnya (untuk deteksi tekan & lepas)
bool lastButtonState[BUTTON_COUNT] = {false};

// Buffer pembacaan serial
String inputString = "";

// =========================================================================
// HITUNG CRC-8 (Polinomial 0x07)
// =========================================================================
byte calculateCRC8(const String &data) {
  byte crc = 0x00;
  for (int i = 0; i < data.length(); i++) {
    crc ^= (byte)data[i];
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

// =========================================================================
// PROSES PACKET & UPDATE STATUS KEYBOARD
// =========================================================================
void processPacket(String packet) {
  packet.trim(); // Buang \r\n

  // Format valid: 10 digit + '*' + 2 digit hex (Contoh: "0000000000*C0")
  int starIndex = packet.indexOf('*');
  if (starIndex == -1) return;

  String payload = packet.substring(0, starIndex);
  String receivedHex = packet.substring(starIndex + 1);

  if (payload.length() != BUTTON_COUNT) return;

  // Verifikasi Checksum CRC-8
  byte computedCRC = calculateCRC8(payload);
  char computedHex[3];
  sprintf(computedHex, "%02X", computedCRC);

  if (receivedHex != String(computedHex)) {
    Serial.println("[Warning] Checksum Error! Paket diabaikan.");
    return;
  }

  // Cek apakah keyboard terhubung/aktif
  bool isConnected = false;
  #if defined(MODE_BLUETOOTH)
    isConnected = keyboard.isConnected();
  #else
    isConnected = true;
  #endif

  // Periksa setiap tombol satu per satu
  for (byte i = 0; i < BUTTON_COUNT; i++) {
    bool isPressed = (payload[i] == '1');

    // Jika terjadi perubahan status pada tombol ke-i
    if (isPressed != lastButtonState[i]) {
      lastButtonState[i] = isPressed;

      if (isConnected) {
        if (isPressed) {
          // Tombol ditekan (Key Down)
          keyboard.press(keyMap[i]);
          Serial.printf(">> Tombol %d DITEKAN (Key: %c)\n", i + 1, keyMap[i]);
        } else {
          // Tombol dilepas (Key Up)
          keyboard.release(keyMap[i]);
          Serial.printf("<< Tombol %d DILEPAS (Key: %c)\n", i + 1, keyMap[i]);
        }
      }
    }
  }
}

// =========================================================================
// SETUP
// =========================================================================
void setup() {
  // Serial monitor untuk debugging ke PC
  Serial.begin(115200);
  Serial.println("--- ESP32 KEYBOARD BRIDGE STARTING ---");

  // Serial Hardware 2 untuk menerima data dari Arduino Nano (Baudrate 115200)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Inisialisasi Keyboard
  #if defined(MODE_BLUETOOTH)
    Serial.println("Mengaktifkan Bluetooth Keyboard...");
    keyboard.begin();
    Serial.println("Silakan koneksikan PC ke: 'Dingdong Arcade Controller'");
  #elif defined(MODE_USB_NATIVE)
    Serial.println("Mengaktifkan USB Native Keyboard...");
    keyboard.begin();
    USB.begin();
  #endif

  inputString.reserve(32);
}

// =========================================================================
// LOOP
// =========================================================================
void loop() {
  // Baca data baris dari Arduino Nano via Serial2
  while (Serial2.available()) {
    char inChar = (char)Serial2.read();

    if (inChar == '\n') {
      // Baris selesai diterima, proses sekarang!
      processPacket(inputString);
      inputString = "";
    } else if (inChar != '\r') {
      inputString += inChar;
    }
  }
}
