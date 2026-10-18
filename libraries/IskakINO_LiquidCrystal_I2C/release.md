# 🚀 Release Notes: IskakINO_LiquidCrystal_I2C v1.1.0

**IskakINO_LiquidCrystal_I2C v1.1.0** hadir dengan pembaruan besar yang berfokus pada **fitur visual non-blocking (asynchronous)**, **optimasi memori RAM**, **pustaka ikon siap pakai**, dan **peningkatan kompatibilitas multi-platform**.

---

## ✨ Fitur Baru (New Features)

### 1. ⏱️ Non-Blocking Animation Ecosystem
Kini Anda dapat membuat efek animasi teks dan timer display tanpa memblokir (`delay()`) proses utama mikrokontroler:
* **Non-Blocking Typewriter**: `typewriterStart(text, row, delayTime)`, `typewriterStop()`, `isTypewriterActive()`.
* **Auto Horizontal Text Scroll**: `scrollTextStart(text, row, delayTime, pauseAtEnd)`, `scrollTextStop()`, `isScrollActive()` untuk menampilkan teks panjang yang bergulir otomatis.
* **Backlight Auto-Timeout**: `setBacklightTimeout(ms)` — lampu latar otomatis mati setelah periode idle tertentu untuk menghemat daya.
* **`lcd.update()`**: Handler terpusat yang dipanggil di `loop()` untuk mengelola seluruh eksekusi waktu non-blocking.

### 2. 📊 Built-in Progress Bar
* **`drawProgressBar(percent, row)`**: Menampilkan progress bar grafis otomatis (0–100%) dengan bar halus memanfaatkan *custom character* slot 7.

### 3. 🎨 Header Preset Ikon Karakter (`IskakINO_LCD_Icons.h`)
Header baru berisi kumpulan array *custom character* 5x8 siap pakai:
* `ICON_WIFI`
* `ICON_BATTERY_FULL`, `ICON_BATTERY_EMPTY`
* `ICON_ARROW_UP`, `ICON_ARROW_DOWN`
* `ICON_BELL`, `ICON_HEART`, `ICON_CHECK`, `ICON_CROSS`
* `ICON_DEGREE` (simbol °), `ICON_THERMOMETER`, `ICON_CLOCK`

### 4. 🖨️ Formatted Print (`printf`-style)
* **`printFormatted(format, ...)`**: Kemudahan menampilkan teks terformat (contoh: `lcd.printFormatted("Temp: %d C", suhu)`). Ukuran buffer dapat disesuaikan melalui `#define LCD_PRINTF_BUFFER_SIZE`.

---

## ⚡ Optimasi & Performa (Improvements)

* **RAM Optimization**: Menambahkan overload `const char*` dan `const String&` pada fungsi teks (`printCenter`, `typewriter`, dll.) untuk menghindari duplikasi heap memori pada papan bersumber daya terbatas seperti Arduino Uno/Nano (ATmega328P).
* **Static I2C Bus Management**: Inisialisasi bus I2C kini bersifat *static per-class*, mencegah konflik dan re-inisialisasi ganda saat memakai lebih dari satu display LCD pada bus yang sama.
* **Custom Pin Support (ESP32/ESP8266)**: Penambahan helper static `LiquidCrystal_I2C::useExternalWireBegin()` untuk skenario kustom pin SDA/SCL.

---

## 🐛 Perbaikan Bug (Bug Fixes)

* Memperbaiki potensi penimpaan konfigurasi pin custom pada ESP32 ketika `Wire.begin(SDA, SCL)` dipanggil secara manual sebelum `lcd.begin()`.
* Memperbaiki contoh `10_LogoAnimation` agar splash screen bawaan tidak muncul dua kali dengan menerapkan `#define ISKAKINO_NO_SPLASH`.

---

## 📂 Contoh Baru (New Examples)

* **`examples/12_NewFeatures_v110`**: Sketch demo komprehensif yang mendemonstrasikan seluruh fitur baru v1.1.0 secara interaktif.
* Pembaruan menyeluruh pada contoh `01` s/d `11` untuk standar penulisan terbaru.

---

## 🔒 Kompatibilitas & Catatan Migrasi

* **100% Backward Compatible**: Seluruh sketsa yang ditulis untuk versi 1.0.x tetap berjalan normal tanpa perubahan kode.
* **Custom Character Slot 7**: Fungsi `drawProgressBar()` mencadangkan slot CGRAM ke-7 (`LCD_PROGRESSBAR_CHAR_LOC`). Hindari menimpa slot ini dengan `createChar(7, ...)` jika menggunakan fitur progress bar.

---

### 📦 Cara Pemasangan / Update
1. Melalui **Arduino Library Manager**: Cari `IskakINO_LiquidCrystal_I2C` lalu klik **Update**.
2. Manual via ZIP: Unduh rilis dari [GitHub Repository](https://github.com/iskakfatoni/IskakINO_LiquidCrystal_I2C/releases/tag/1.1.0) lalu pasang melalui menu **Sketch -> Include Library -> Add .ZIP Library**.
