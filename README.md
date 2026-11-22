# TOMBOL DINGDONG - Arcade Controller & Custom Casing

Proyek kontroler tombol dingdong/arcade berbasis **Arduino Nano** dengan firmware responsif dan file desain **3D Casing** siap cetak.

---

## 📁 Struktur Direktori

```text
TOMBOL_DINGDONG/
├── 3d_casing/                          # File desain dan cetak 3D Casing
│   ├── FreeCAD/                        # File master 3D CAD (.FCStd)
│   │   ├── CASING_NANO_ASSEMBLY.FCStd
│   │   ├── CASING_NANO_ATAS.FCStd
│   │   └── CASING_NANO_BAWAH.FCStd
│   ├── STL/                            # File mesh siap cetak 3D Printer (.stl)
│   │   ├── CASING_NANO_ATAS.stl
│   │   ├── CASING_NANO_BAWAH.stl
│   │   └── original/                   # Backup model STL versi awal
│   │       ├── CASING_NANO_ATAS_ORIGINAL.stl
│   │       └── CASING_NANO_BAWAH_ORIGINAL.stl
│   ├── STEP/                           # Format universal CAD (.step)
│   │   ├── CASING_NANO_ATAS.step
│   │   └── CASING_NANO_BAWAH.step
│   └── generate_casing.py              # Script Python generator FreeCAD
├── digital_dingdong_controller/        # Firmware Arduino Nano (10 Tombol Dingdong)
│   └── digital_dingdong_controller.ino
├── esp32_keyboard_bridge/              # Firmware ESP32 Bridge (UART -> USB/BLE Keyboard)
│   └── esp32_keyboard_bridge.ino
├── .gitignore                          # Filter file temporary / backup
└── README.md                           # Dokumentasi proyek
```

---

## ⚡ Fitur Firmware (`digital_dingdong_controller`)

* **2 Mode Pengiriman (Dapat Dipilih):**
  * **Mode 2 (Default - `MODE_EVENT_DRIVEN`):** Mode hening. Hanya mengirim data 1 baris seketika saat ada tombol yang **ditekan** atau **dilepas**. Saat menganggur / ditahan, Arduino diam total (zero spam / hemat resource).
  * **Mode 1 (`MODE_SERIAL_RAW`):** Mode streaming berkala. Mengirim cepat tiap 10 ms saat tombol aktif, dan melambat ke 500 ms (detak jantung / *heartbeat*) saat menganggur.
* **Debounce Non-Blocking (20 ms):** Menggunakan `millis()` independen di setiap pin untuk menyaring getaran mekanik microswitch arcade (*anti double-click*).
* **Format Data Aman (CRC-8 Checksum + Enter):** Mengirimkan 10 karakter status tombol murni dari byte pertama, separator `*`, 2 digit Hex CRC-8 checksum untuk validasi data presisi per tombol, dan diakhiri karakter Enter (`\r\n`).
* **Active-LOW:** Menggunakan resistor internal `INPUT_PULLUP` Arduino (kaki tombol cukup disambung ke Pin Arduino dan GND).
* **Kompatibilitas Game:** Protokol dan urutan pin ini kompatibel langsung secara native dengan game arcade HTML5/Electron (seperti *Witch's Ritual Arcade / booth-withces*).

---

## 🔌 Pin Mapping (10 Tombol)

| Byte ke- | Pin Arduino Nano | Logika Aktif | Status |
| :---: | :---: | :---: | :---: |
| 1 | **A4** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 2 | **A3** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 3 | **A2** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 4 | **A1** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 5 | **A0** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 6 | **D9** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 7 | **D8** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 8 | **D7** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 9 | **D6** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 10 | **D5** | LOW (ke GND) | `'1'` (tekan) / `'0'` (lepas) |
| 11 | **`*`** | Pembatas | Separator Checksum |
| 12 & 13 | **Checksum** | HEX (2 digit) | Hasil CRC-8 dari 10 karakter status tombol |
| 14 & 15 | **Enter** | `\r\n` (CR + LF) | Akhiran paket |

---

## 📡 Protokol Serial

* **Baudrate:** `115200 bps`
* **Format Paket:** `[10 Status Tombol]*[Checksum 2 Digit HEX]\r\n`
* **Contoh Data Diterima di Komputer:**
  * Semua tombol lepas: `0000000000*C0\r\n`
  * Tombol 1 (A4) ditekan: `1000000000*A8\r\n`
  * Tombol 2 (A3) ditekan: `0100000000*B9\r\n`
  * Tombol 10 (D5) ditekan: `0000000001*C7\r\n`

---

## 💻 Panduan Ekstraksi & Validasi Checksum (Sisi Host/PC)

Untuk memastikan data yang diterima tidak korup akibat gangguan kabel USB atau noise, aplikasi di PC/komputer penerima harus:
1. Membaca data per baris sampai menemukan karakter `\r\n` (*Enter*).
2. Memisahkan string menggunakan tanda pemisah `*`.
   * Bagian depan (10 karakter): **Payload status tombol**.
   * Bagian belakang (2 karakter): **Checksum yang diterima**.
3. Menghitung ulang **CRC-8** dari 10 karakter payload (menggunakan Polinomial `0x07`).
4. Jika **Checksum Hitung == Checksum Diterima**, maka data **valid** dan status tombol aman untuk diproses.

---

### Contoh Implementasi 1: Python

Cocok jika Anda membuat game dengan Pygame, emulator, atau script virtual controller (vJoy / keyboard emulator):

```python
import serial

def calculate_crc8(payload: str) -> str:
    """Menghitung CRC-8 dengan Polinomial 0x07"""
    crc = 0x00
    for char in payload:
        crc ^= ord(char)
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return f"{crc:02X}"

def parse_packet(raw_line: str):
    """
    Ekstraksi dan validasi paket serial dari Arduino
    Format: '0000000000*C0'
    """
    clean_line = raw_line.strip()  # Buang \r\n
    if "*" not in clean_line:
        return None

    # Pisahkan payload dan checksum
    parts = clean_line.split("*")
    if len(parts) != 2:
        return None

    payload, received_crc = parts[0], parts[1].upper()

    # Validasi panjang payload (10 tombol)
    if len(payload) != 10:
        return None

    # Verifikasi Checksum
    computed_crc = calculate_crc8(payload)
    if computed_crc == received_crc:
        # Data Valid! Ubah string '1'/'0' menjadi array boolean/integer
        button_states = [char == '1' for char in payload]
        return button_states
    else:
        print(f"[Warning] Checksum mismatch! Diterima: {received_crc}, Dihitung: {computed_crc}")
        return None

# Contoh Penggunaan Membaca Port Serial:
if __name__ == "__main__":
    # Ganti 'COM3' (Windows) atau '/dev/ttyUSB0' (Linux) sesuai port Arduino Anda
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    print("Mendengarkan data serial dari Dingdong Controller...")

    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore')
            if line:
                buttons = parse_packet(line)
                if buttons is not None:
                    # buttons adalah list berisi 10 boolean: [True/False, ...]
                    print(f"Status Tombol: {buttons}")
                    if buttons[0]: # Tombol 1 (A4)
                        print(">> Tombol 1 Aktif!")
        except KeyboardInterrupt:
            ser.close()
            break
```

---

### Contoh Implementasi 2: Node.js / JavaScript

Cocok untuk game berbasis web, Electron, atau Node.js serialport:

```javascript
// Fungsi kalkulasi CRC-8 (Polinomial 0x07)
function calculateCRC8(payload) {
  let crc = 0x00;
  for (let i = 0; i < payload.length; i++) {
    crc ^= payload.charCodeAt(i);
    for (let b = 0; b < 8; b++) {
      if (crc & 0x80) {
        crc = ((crc << 1) ^ 0x07) & 0xff;
      } else {
        crc = (crc << 1) & 0xff;
      }
    }
  }
  return crc.toString(16).toUpperCase().padStart(2, '0');
}

// Fungsi Parsing Paket
function parsePacket(line) {
  const cleanLine = line.trim();
  const [payload, receivedCrc] = cleanLine.split('*');

  if (!payload || !receivedCrc || payload.length !== 10) {
    return null; // Format tidak sesuai
  }

  const computedCrc = calculateCRC8(payload);
  if (computedCrc === receivedCrc.toUpperCase()) {
    // Valid! Mengembalikan status 10 tombol dalam bentuk array boolean
    return payload.split('').map(ch => ch === '1');
  } else {
    console.warn(`Checksum Corrupt! Diterima: ${receivedCrc}, Dihitung: ${computedCrc}`);
    return null;
  }
}

// Contoh Pengujian:
const sample = "1000000000*A8";
const buttons = parsePacket(sample);
console.log("Hasil Validasi:", buttons);
// Output: [ true, false, false, false, false, false, false, false, false, false ]
```

---

## 🛠️ Panduan Upload Arduino Nano

1. Buka file `digital_dingdong_controller/digital_dingdong_controller.ino` di Arduino IDE.
2. Pilih Board: **Tools $\rightarrow$ Board $\rightarrow$ Arduino AVR Boards $\rightarrow$ Arduino Nano**.
3. Pilih Processor: **Tools $\rightarrow$ Processor $\rightarrow$ ATmega328P (Old Bootloader)** *(khusus Nano clone/CH340)*.
4. Pilih Port COM yang sesuai.
5. Klik **Upload**.

---

## 🎮 Opsi Jembatan ESP32 (Keyboard Mode)

Jika kontroler ini ingin dipakai untuk emulator atau game PC yang **hanya menerima input Keyboard biasa** (tanpa membongkar wiring pada Arduino Nano):

1. Buka file `esp32_keyboard_bridge/esp32_keyboard_bridge.ino` di Arduino IDE.
2. Pilih mode output di baris atas kodingan:
   * `#define MODE_BLUETOOTH` : Untuk ESP32 WROOM biasa (menjadi **Bluetooth Wireless Keyboard** tanpa kabel ke PC).
   * `#define MODE_USB_NATIVE` : Untuk ESP32-S2 / ESP32-S3 (menjadi **USB Keyboard kabel** langsung ke PC).
3. Sambungkan 3 kabel dari Arduino Nano ke ESP32:
   * **Nano Pin D1 (TX)** $\rightarrow$ **ESP32 Pin GPIO 16 (RX2)**
   * **Nano Pin GND** $\rightarrow$ **ESP32 Pin GND**
   * **Nano Pin 5V** $\rightarrow$ **ESP32 Pin VIN / 5V**
4. Kontroler akan langsung dikenali oleh Windows/Linux/Mac sebagai Keyboard multi-tekan (simultan).

---

## 🖨️ Desain 3D Casing

* File STL siap cetak berada di folder `3d_casing/STL/`.
* Dilengkapi lubang mur hex M3 (*nut trap*) dan clearance presisi untuk Arduino Nano.
* Didesain menggunakan FreeCAD (script generator otomatis tersedia di `3d_casing/generate_casing.py`).
