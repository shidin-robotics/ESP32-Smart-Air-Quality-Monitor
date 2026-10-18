# IskakINO_LiquidCrystal_I2C

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-Arduino%20%7C%20ESP32-green)](#)
[![License](https://img.shields.io/badge/license-MIT-lightgrey)](LICENSE)
![Version](https://img.shields.io/github/v/release/iskakfatoni/IskakINO_LiquidCrystal_I2C?color=blue)
[![CI](https://github.com/iskakfatoni/IskakINO_LiquidCrystal_I2C/actions/workflows/arduino-ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO_LiquidCrystal_I2C/actions)

LiquidCrystal I2C library for **HD44780 LCD** using **PCF8574 I2C expander**  
with **automatic I2C address detection**.

Designed to be:
- ✅ Backward compatible with `LiquidCrystal_I2C`
- ⚡ Lightweight & optimized for flash/RAM
- 🔌 Arduino & ESP32 friendly
- 🧼 Clean API & maintainer-grade structure
- 🧵 Non-blocking friendly untuk efek teks & animasi (v1.1.0+)
---
## 📑 Table of Contents
- [Features](#-features)
- [Supported Hardware](#-supported-hardware)
- [Installation](#-installation)
- [Wiring](#-wiring)
- [Basic Usage](#-basic-usage)
- [Showcase Examples](#-showcase-examples)
- [API Compatibility](#-api-compatibility)
- [Versioning](#-versioning)
- [License](#-license)

---
## ✨ Features
- Automatic I2C address scanning (0x20–0x27)
- Full **HD44780 command support**
- Standard `LiquidCrystal` compatible API
- Custom character (CGRAM) support
- Scroll, autoscroll, cursor & blink control
- Non-blocking friendly (millis-based design)
- Non-blocking typewriter & auto horizontal scroll text effect (v1.1.0)
- Backlight auto-timeout (v1.1.0)
- Built-in progress bar (v1.1.0)
- printf-style formatted print (v1.1.0)
- Preset icon set (wifi, battery, arrow, bell, heart, dll) via `IskakINO_LCD_Icons.h` (v1.1.0)
- Safe multi-LCD initialization on a shared I2C bus (v1.1.0)
- Optimized for ESP32 & Arduino boards

---
## 🧩 Supported Hardware
This library supports HD44780 LCDs with I2C backpack (PCF8574 / PCF8574A) and popular microcontrollers:
| Board / MCU | Notes |
|-------------|-------|
| Arduino UNO / Nano | 5V logic, SDA/A4, SCL/A5 |
| Arduino Mega | 5V logic, SDA/20, SCL/21 |
| ESP32 | 3.3V logic, SDA = GPIO21, SCL = GPIO22 |
| ESP32-C3 | 3.3V logic, SDA = GPIO20, SCL = GPIO21 |
| ESP8266 (NodeMCU / Wemos D1) | 3.3V logic, SDA = D2 (GPIO4), SCL = D1 (GPIO5) |

---
## 📦 Installation
### Arduino IDE (Library Manager)
1. Open **Arduino IDE**
2. Sketch → Include Library → **Manage Libraries**
3. Search for **IskakINO_LiquidCrystal_I2C**
4. Click **Install**
### Manual Installation (ZIP)
1. Download this repository as ZIP
2. Arduino IDE → Sketch → Include Library → **Add .ZIP Library**

---
## 🔌 Wiring
Connect your HD44780 LCD with I2C backpack to your board:
| LCD I2C Pin | Arduino | ESP32 | ESP32-C3 | ESP8266 |
|------------|----------|--------|-----------|----------|
| SDA | A4 | GPIO21 | GPIO20 | D2 (GPIO4) |
| SCL | A5 | GPIO22 | GPIO21 | D1 (GPIO5) |
| VCC | 5V | 3.3V | 3.3V | 3.3V |
| GND | GND | GND | GND | GND |
> ⚠ Ensure voltage compatibility between your LCD backpack and board.

---
## 🚀 Basic Usage
Cukup gunakan kode sederhana ini untuk memulai. Library akan secara otomatis mencari alamat I2C LCD Anda (0x27 atau 0x3F), sehingga Anda tidak perlu mengaturnya secara manual.

```cpp
#include <IskakINO_LiquidCrystal_I2C.h>

// Inisialisasi: (Jumlah Kolom, Jumlah Baris)
// Alamat I2C akan dideteksi secara otomatis!
LiquidCrystal_I2C lcd(16, 2);

void setup() {
  // Memulai komunikasi I2C dan LCD
  lcd.begin();
  
  // Menyalakan lampu latar
  lcd.backlight();

  // Menampilkan pesan di baris pertama
  lcd.setCursor(0, 0);
  lcd.print("Hello IskakINO");
  
  // Menampilkan pesan di baris kedua
  lcd.setCursor(0, 1);
  lcd.print("Auto-Scan Ready");
}

void loop() {
  // Kode utama Anda di sini
}
```
## Mengganti Alamat I2C Saat Runtime
Jika Anda punya beberapa LCD atau ingin mengganti alamat secara dinamis,
cukup panggil `setAddress()` kapan saja — bahkan setelah `begin()`:

```cpp
lcd.begin();          // LCD pertama, alamat dideteksi otomatis (auto-scan)
lcd.setAddress(0x3F); // Ganti ke alamat lain, LCD otomatis di-reinit
```

## ⚡ Fitur Non-Blocking (v1.1.0+)
Untuk efek animasi (typewriter, scroll teks, backlight timeout) tanpa memblokir
`loop()` Anda, panggil `lcd.update()` **satu kali di setiap `loop()`**:

```cpp
#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

void setup() {
  lcd.begin();
  lcd.setBacklightTimeout(15000); // backlight mati otomatis setelah 15 detik idle
  lcd.typewriterStart("Hello IskakINO!", 0, 80);
}

void loop() {
  lcd.update(); // WAJIB dipanggil tiap loop() untuk fitur non-blocking

  // Kode lain Anda tetap bisa berjalan di sini tanpa terganggu delay()
}
```

Fungsi non-blocking yang tersedia:
| Fungsi | Deskripsi |
|:---|:---|
| `typewriterStart(text, row, delayTime)` | Mulai animasi mengetik non-blocking |
| `typewriterStop()` / `isTypewriterActive()` | Hentikan / cek status animasi typewriter |
| `scrollTextStart(text, row, intervalMs)` | Mulai teks berjalan otomatis (untuk teks > lebar layar) |
| `scrollTextStop()` / `isScrollActive()` | Hentikan / cek status scroll |
| `setBacklightTimeout(ms)` | Backlight mati otomatis setelah idle sekian ms (0 = nonaktif) |
| `update()` | Wajib dipanggil tiap `loop()` untuk menjalankan fitur di atas |

----
## 📂 Showcase Examples
Library ini menyediakan pembelajaran bertahap dari tingkat dasar hingga mahir. Anda dapat menemukan file-file ini di folder `examples/`.
### Daftar Contoh Berdasarkan Level
| No | Folder Contoh | Deskripsi | Level |
|:---:|:---|:---|:---:|
| 01 | **[01_HelloWorld](./examples/01_HelloWorld)** | Inisialisasi LCD dan fitur Auto-Scan alamat I2C secara otomatis. | ⭐ Basic |
| 02 | **[02_BlinkAndCursor](./examples/02_BlinkAndCursor)** | Mengontrol visual kursor (garis bawah) dan efek kedipan (blink). | ⭐ Basic |
| 03 | **[03_ScrollAndBacklight](./examples/03_ScrollAndBacklight)** | Animasi teks berjalan ke samping dan kontrol daya lampu latar. | ⭐ Basic |
| 04 | **[04_CustomCharacter](./examples/04_CustomCharacter)** | Cara menggambar simbol buatan sendiri (ikon hati, bel, dll). | ⭐⭐ Interm. |
| 05 | **[05_ProgressBar](./examples/05_ProgressBar)** | Progress bar dinamis memakai fungsi built-in `drawProgressBar()`. | ⭐⭐ Interm. |
| 06 | **[06_I2CScannerMode](./examples/06_I2CScannerMode)** | Menggunakan fungsi internal library untuk mendeteksi perangkat I2C lain. | ⭐⭐ Interm. |
| 07 | **[07_DigitalClock](./examples/07_DigitalClock)** | Membuat jam digital akurat tanpa `delay()` menggunakan `millis()`. | ⭐⭐⭐ Advance |
| 08 | **[08_MenuNavigation](./examples/08_MenuNavigation)** | Struktur menu multi-halaman untuk navigasi antarmuka pengguna (UI). | ⭐⭐⭐ Advance |
| 09 | **[09_CustomI2CPins_ESP32](./examples/09_CustomI2CPins_ESP32)** | Konfigurasi pin SDA & SCL kustom untuk ESP32/ESP8266 (pakai `useExternalWireBegin()`). | ⭐⭐⭐ Advance |
| 10 | **[10_LogoAnimation](./examples/10_LogoAnimation)** | Membuat animasi Splash Screen menggunakan gabungan teknik kustom. | ⭐⭐⭐ Advance |
| **---** | **--- NEW IN v1.0.1 ---** | **-------------------------------------------------------** | **---** |
| 11 | **[11_NewFeatures_v101](./examples/11_NewFeatures_v101)** | Demo fitur `printCenter`, `typewriter`, dan `isConnected`. | ⭐⭐ Interm. |
| **---** | **--- NEW IN v1.1.0 ---** | **-------------------------------------------------------** | **---** |
| 12 | **[12_NewFeatures_v110](./examples/12_NewFeatures_v110)** | Demo non-blocking typewriter, auto-scroll, progress bar, backlight timeout, printFormatted, dan preset ikon. | ⭐⭐⭐ Advance |

---
### 🚀 Cara Menjalankan Contoh
1. **Instalasi**: Pastikan folder library `IskakINO_LiquidCrystal_I2C` sudah berada di dalam folder `libraries` Arduino Anda.
2. **Buka IDE**: Jalankan Arduino IDE.
3. **Pilih Contoh**: Klik menu **File** > **Examples** > **IskakINO_LiquidCrystal_I2C**.
4. **Upload**: Pilih contoh (mulai dari `01_HelloWorld`), lalu klik tombol **Upload**.
> **Catatan Hardware:**
> - Untuk **Arduino Uno**, sambungkan SDA ke `A4` dan SCL ke `A5`.
> - Untuk **ESP32**, pin default adalah `GPIO 21 (SDA)` dan `GPIO 22 (SCL)`.
> - Pastikan Jumper I2C pada modul belakang LCD terpasang agar lampu latar (backlight) menyala.

---

## 📖 Dokumentasi API (API Reference)

Berikut adalah daftar fungsi utama yang tersedia di **IskakINO_LiquidCrystal_I2C v1.1.0**.
### 1. Inisialisasi & Kontrol Dasar
* **`begin()`**
    Inisialisasi LCD dan otomatis mencari alamat I2C (Auto-Scan). Tidak perlu memasukkan alamat secara manual.
* **`backlight()` / `noBacklight()`**
    Menyalakan atau mematikan lampu latar (backlight) LCD.
* **`setBacklightTimeout(unsigned long ms)`** *(v1.1.0)*
    Backlight otomatis mati setelah idle sekian milidetik (0 = nonaktif). Butuh `update()` di `loop()`.
* **`clear()`**
    Menghapus seluruh tampilan layar dan mereset posisi kursor ke pojok kiri atas (0,0).
* **`setCursor(col, row)`**
    Menentukan posisi kursor sebelum mencetak teks.
* **`setAddress(uint8_t addr)` / `getAddress()`**
  Mengatur atau membaca alamat I2C LCD secara manual. Bisa dipanggil kapan saja —
  jika dipanggil **setelah** `begin()`, LCD akan otomatis di-reinit dengan alamat
  baru tanpa perlu memanggil `begin()` ulang secara manual.
* **`static useExternalWireBegin()`** *(v1.1.0)*
    Panggil setelah `Wire.begin(SDA, SCL)` manual dan sebelum `lcd.begin()`, supaya
    library tidak menimpa konfigurasi pin custom Anda. Lihat contoh `09_CustomI2CPins_ESP32`.
* **`update()`** *(v1.1.0)*
    Wajib dipanggil sekali tiap `loop()` untuk menjalankan fitur non-blocking
    (`typewriterStart`, `scrollTextStart`, `setBacklightTimeout`).

### 2. Fitur Cerdas (Smart UX) ✨
Fungsi-fungsi ini dirancang untuk mempermudah pembuatan antarmuka pengguna tanpa logika matematika yang rumit.
| Fungsi | Parameter | Deskripsi |
|:---|:---|:---|
| **`printCenter`** | `(const char* / String text, int row)` | Menampilkan teks otomatis di tengah baris yang ditentukan. Overload `const char*` tersedia sejak v1.1.0 untuk hemat RAM. |
| **`typewriter`** | `(const char* / String text, int row, int delayTime)` | Efek animasi mengetik versi **blocking** (perilaku lama, tetap didukung). |
| **`typewriterStart` / `typewriterStop` / `isTypewriterActive`** *(v1.1.0)* | `(text, row, delayTime)` | Versi **non-blocking** dari `typewriter()`. Jalankan bersama `update()` di `loop()`. |
| **`scrollTextStart` / `scrollTextStop` / `isScrollActive`** *(v1.1.0)* | `(text, row, intervalMs)` | Teks panjang berjalan otomatis secara horizontal, non-blocking. |
| **`drawProgressBar`** *(v1.1.0)* | `(uint8_t percent, uint8_t row)` | Progress bar built-in 0–100%, memakai custom character slot 7. |
| **`printFormatted`** *(v1.1.0)* | `(const char* format, ...)` | Print bergaya `printf`, contoh: `lcd.printFormatted("Suhu: %d C", 27);` |
| **`isConnected`** | `None` | Mengembalikan `true` jika hardware LCD terdeteksi di jalur I2C. |

### 3. Custom Characters & Grafik
Anda dapat membuat hingga 8 karakter buatan sendiri (ikon, logo, dll).
* **`createChar(uint8_t location, uint8_t charmap[])`**: Menyimpan karakter ke memori LCD (index 0-7). *Catatan: slot 7 dicadangkan otomatis oleh `drawProgressBar()` jika fitur itu dipakai.*
* **`write(uint8_t location)`**: Menampilkan karakter kustom yang telah disimpan.
* **`IskakINO_LCD_Icons.h`** *(v1.1.0)*: Header opsional berisi preset ikon siap pakai (wifi, baterai, panah, lonceng, hati, check, cross, termometer, jam). Tinggal `#include <IskakINO_LCD_Icons.h>` lalu `lcd.createChar(slot, ICON_WIFI);` dsb.


---
## 🔌 Referensi Koneksi (Pinout)
Untuk memastikan library berjalan lancar, pastikan koneksi pin I2C sesuai dengan board yang Anda gunakan:
| Platform | SDA Pin | SCL Pin | Tegangan (VCC) |
|:---|:---:|:---:|:---|
| **Arduino Uno / Nano** | A4 | A5 | 5V |
| **ESP8266 (NodeMCU)** | D2 (GPIO4) | D1 (GPIO5) | 3.3V / 5V |
| **ESP32 (Wroom)** | GPIO21 | GPIO22 | 3.3V / 5V |

> [!TIP]
> **Khusus ESP32:** Jika Anda menggunakan pin I2C selain default, panggil `Wire.begin(SDA_PIN, SCL_PIN);` di dalam `setup()`, lalu panggil `LiquidCrystal_I2C::useExternalWireBegin();`, baru kemudian panggil `lcd.begin();`. Lihat contoh `09_CustomI2CPins_ESP32`.

---
## 🔁 API Compatibility
Library ini dirancang agar **100% kompatibel** dengan pola penggunaan `LiquidCrystal_I2C` yang umum digunakan di komunitas Arduino. Anda tidak perlu mengubah logika kode lama Anda.
### 1. Auto-Detect Mode (Rekomendasi)
Cukup masukkan ukuran layar. Library akan melakukan scanning otomatis pada jalur I2C untuk menemukan alamat LCD Anda.
```cpp
// Format: (Kolom, Baris)
LiquidCrystal_I2C lcd(16, 2);
```
2. Standard Mode (Manual Address)
Jika Anda ingin menentukan alamat I2C secara spesifik (misalnya jika Anda memiliki lebih dari satu LCD), library tetap mendukung cara lama.
```cpp
// Format: (Alamat_I2C, Kolom, Baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);
```
3. Kompatibilitas Fungsi Utama
Semua fungsi standar tetap tersedia dan berjalan normal:
```cpp
lcd.init() atau lcd.begin()
lcd.backlight() / lcd.noBacklight()
lcd.createChar()
lcd.scrollDisplayLeft() / lcd.scrollDisplayRight()
```
Catatan: Panggilan API lama (Legacy) tetap berfungsi penuh tanpa memerlukan modifikasi pada sketsa Anda.

---
## 🏷 Riwayat Versi (Versioning)
Library ini mengikuti standar **[Semantic Versioning](https://semver.org/)** (`MAJOR.MINOR.PATCH`).
| Versi | Status | Deskripsi Perubahan |
|:---:|:---:|:---|
| **v1.1.0** | **Latest** | **Non-Blocking & UX Update**: Typewriter & scroll text non-blocking (`typewriterStart`, `scrollTextStart`, `update()`), backlight auto-timeout, progress bar built-in, `printFormatted()`, overload `const char*`, preset ikon (`IskakINO_LCD_Icons.h`), dan inisialisasi multi-LCD yang lebih aman via `useExternalWireBegin()`. |
| **v1.0.3** | Stable | **Stability Fix**: `setAddress()` kini bisa dipanggil kapan saja (sebelum atau sesudah `begin()`) dan otomatis re-init LCD dengan alamat baru. Sebelumnya alamat baru diabaikan jika dipanggil setelah `begin()`. |
| **v1.0.1** | Stable | **Smart UX Update**: Penambahan fungsi `printCenter()`, `typewriter()`, `isConnected()`, dan perbaikan stabilitas compile pada ESP8266. |
| **v1.0.0** | Stable | **Initial Release**: Fitur Auto-Address, dukungan karakter kustom, dan kompatibilitas penuh dengan API LiquidCrystal_I2C standar. |
### ✅ Komitmen Stabilitas
* **Backward Compatibility**: API standar akan selalu didukung untuk memastikan proyek lama Anda tetap berjalan.
* **No Breaking Changes**: Tidak akan ada perubahan drastis pada fungsi inti dalam seri versi `1.x.x`.
* **Cross-Platform**: Setiap rilis divalidasi via GitHub Actions untuk Arduino Uno, ESP32, dan ESP8266.




---
## 🤝 Kontribusi (Contributing)
Kontribusi selalu terbuka bagi siapa saja! Jika Anda menemukan bug atau memiliki ide fitur baru, silakan:
1. **Fork** repository ini.
2. Buat branch baru (`git checkout -b fitur-baru`).
3. Lakukan **Commit** perubahan Anda (`git commit -m 'Menambah fitur X'`).
4. **Push** ke branch tersebut (`git push origin fitur-baru`).
5. Buat **Pull Request**.
Kami sangat menghargai masukan untuk pengembangan library **IskakINO** agar lebih baik lagi bagi komunitas Maker di Indonesia.

---
## 📄 Lisensi (License)
Library ini didistribusikan di bawah **Lisensi MIT**. Anda bebas menggunakan, memodifikasi, dan membagikan library ini baik untuk proyek hobi maupun komersial selama tetap menyertakan atribusi penulis asli.
Copyright (c) 2026 **Iskak Fatoni**

---
## ☕ Dukungan
Jika library ini membantu proyek Anda, jangan ragu untuk memberikan **Star ⭐** pada repository ini atau bagikan ke teman-teman sesama pengembang. Dukungan Anda sangat berarti untuk pengembangan library gratis lainnya di masa depan!
**Happy Coding! 🚀**
