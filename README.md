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
├── digital_dingdong_controller/        # Firmware Arduino Nano (Versi Digital)
│   └── digital_dingdong_controller.ino
├── .gitignore                          # Filter file temporary / backup
└── README.md                           # Dokumentasi proyek
```

---

## ⚡ Fitur Firmware (`digital_dingdong_controller`)

* **Debounce Non-Blocking (20 ms):** Menggunakan `millis()` independen di setiap pin untuk menyaring getaran mekanik microswitch arcade (*anti double-click*).
* **Mode Cepat (10 ms / 100 Hz):** Mengirimkan data secara halus dan ultra-responsif ke PC saat ada tombol yang ditekan atau ditahan.
* **Keep-Alive / Heartbeat Otomatis (1000 ms):** Jika mesin tidak disentuh lebih dari 3 detik (*idle*), interval pengiriman otomatis melambat menjadi 1 detik sekali agar tidak membebani port serial komputer.
* **Format Data Aman (CRC-8 Checksum + Enter):** Mengirimkan 10 karakter status tombol murni dari byte pertama, separator `*`, 2 digit Hex CRC-8 checksum untuk validasi data presisi per tombol, dan diakhiri karakter Enter (`\r\n`).
* **Active-LOW:** Menggunakan resistor internal `INPUT_PULLUP` Arduino (kaki tombol cukup disambung ke Pin Arduino dan GND).

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

## 🛠️ Panduan Upload Arduino Nano

1. Buka file `digital_dingdong_controller/digital_dingdong_controller.ino` di Arduino IDE.
2. Pilih Board: **Tools $\rightarrow$ Board $\rightarrow$ Arduino AVR Boards $\rightarrow$ Arduino Nano**.
3. Pilih Processor: **Tools $\rightarrow$ Processor $\rightarrow$ ATmega328P (Old Bootloader)** *(khusus Nano clone/CH340)*.
4. Pilih Port COM yang sesuai.
5. Klik **Upload**.

---

## 🖨️ Desain 3D Casing

* File STL siap cetak berada di folder `3d_casing/STL/`.
* Dilengkapi lubang mur hex M3 (*nut trap*) dan clearance presisi untuk Arduino Nano.
* Didesain menggunakan FreeCAD (script generator otomatis tersedia di `3d_casing/generate_casing.py`).
