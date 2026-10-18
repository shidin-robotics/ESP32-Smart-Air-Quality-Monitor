# Changelog
Semua perubahan penting pada proyek ini akan didokumentasikan dalam file ini.
Format berdasarkan [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mengikuti [Semantic Versioning](https://semver.org/).

---

## [1.1.0] - 2026-07-12

### Ditambahkan (Added)
- **Non-blocking typewriter**: `typewriterStart()` / `typewriterStop()` / `isTypewriterActive()`.
  Dipakai bersama `update()` di `loop()`, tidak lagi memblokir program utama seperti
  `typewriter()` versi lama (yang tetap dipertahankan untuk backward compatibility).
- **Overload `const char*`** untuk `printCenter()` dan `typewriter()`, selain `String`
  yang sudah ada — menghemat RAM di board kecil seperti Uno/Nano.
- **Backlight auto-timeout**: `setBacklightTimeout(ms)` — backlight otomatis mati
  setelah idle sekian milidetik. Aktivitas dihitung dari setiap output ke layar.
- **Auto horizontal scroll**: `scrollTextStart()` / `scrollTextStop()` / `isScrollActive()`
  untuk menampilkan teks yang lebih panjang dari lebar layar secara otomatis, non-blocking.
- **`update()`**: method baru yang WAJIB dipanggil sekali tiap `loop()` untuk menjalankan
  semua fitur non-blocking di atas (typewriter, scroll, backlight timeout).
- **`drawProgressBar(percent, row)`**: progress bar built-in menggunakan custom character
  (slot 7 dicadangkan secara internal untuk fitur ini). Contoh `05_ProgressBar`
  diperbarui untuk memakai fungsi ini, menggantikan implementasi manual sebelumnya.
- **`printFormatted(format, ...)`**: print bergaya printf, mis.
  `lcd.printFormatted("Suhu: %d C", suhu);`. Ukuran buffer internal bisa diatur lewat
  `#define LCD_PRINTF_BUFFER_SIZE` sebelum `#include`.
- **Header baru `IskakINO_LCD_Icons.h`**: kumpulan preset custom character siap pakai
  (wifi, baterai, panah, lonceng, hati, check, cross, termometer, jam, dll).
- **`LiquidCrystal_I2C::useExternalWireBegin()`** *(static)*: dipanggil setelah
  `Wire.begin(SDA, SCL)` manual (mis. custom pin ESP32/ESP8266), sebelum `lcd.begin()`.
- Contoh sketch baru `examples/12_NewFeatures_v110` yang mendemokan seluruh fitur di atas.

### Diubah (Changed)
- Inisialisasi bus I2C (`Wire.begin()`) sekarang dibuat **static per-class**, bukan
  per-instance — mencegah re-inisialisasi bus menimpa konfigurasi pin custom (SDA/SCL)
  ketika memakai lebih dari satu LCD pada bus I2C yang sama.
- Contoh `09_CustomI2CPins_ESP32` diperbarui untuk memanggil `useExternalWireBegin()`
  (lihat catatan bug di bawah).

### Diperbaiki (Fixed)
- **[Regresi internal]** Perubahan static `Wire.begin()` di atas semula akan menimpa
  konfigurasi pin I2C custom pada board ESP32/ESP8266 jika `Wire.begin(SDA, SCL)`
  dipanggil manual oleh user sebelum `lcd.begin()`. Ditemukan saat menyesuaikan
  contoh `09_CustomI2CPins_ESP32`, dan diperbaiki dengan menambahkan
  `useExternalWireBegin()` sebelum rilis — tidak pernah terbit ke publik.
- Contoh `10_LogoAnimation` berpotensi menampilkan splash screen dua kali (bawaan
  library + animasi custom di contoh). Ditambahkan `#define ISKAKINO_NO_SPLASH`
  di contoh tersebut untuk mencegah duplikasi.

### Catatan Kompatibilitas
- Tidak ada breaking change pada API publik. Semua kode v1.0.3 tetap berfungsi
  seperti sebelumnya.
- `drawProgressBar()` mencadangkan custom character slot 7 — hindari
  `createChar(7, ...)` manual jika fitur ini dipakai.
- Jika sketsa Anda memanggil `Wire.begin(SDA, SCL)` secara manual (custom pin
  ESP32/ESP8266), tambahkan `LiquidCrystal_I2C::useExternalWireBegin();` setelahnya
  dan sebelum `lcd.begin()` — lihat contoh `09_CustomI2CPins_ESP32`.

---

## [1.0.3] - 2026-07-08
### Diperbaiki (Fixed)
- `setAddress()` sekarang bisa dipanggil kapan saja, termasuk **setelah** `begin()`
  dijalankan. Sebelumnya, memanggil `setAddress()` setelah `begin()` tidak berefek
  karena guard `if (_initialized) return;` di dalam `begin()` mencegah re-inisialisasi.
  Sekarang, `setAddress()` otomatis mereset status internal dan memanggil ulang
  `begin()` dengan alamat I2C yang baru.
### Diubah (Changed)
- Menambahkan komentar penjelas pada `backlight()` / `noBacklight()` agar lebih
  eksplisit menjelaskan bagaimana bit backlight ditambahkan otomatis oleh
  `_expanderWrite()`. Tidak ada perubahan perilaku fungsional.

---

## [1.0.0] - 2026-02-10
### Ditambahkan (Added)
- Rilis stabil pertama dari IskakINO_LiquidCrystal_I2C
- Deteksi alamat I2C otomatis untuk PCF8574 (0x20–0x27)
- API yang kompatibel dengan library LiquidCrystal_I2C umum lainnya (backward compatible)
- Dukungan kompatibilitas untuk ESP32
- Opsi konfigurasi saat kompilasi (compile-time configuration)
- Dukungan debug serial opsional
- Contoh sketch dasar beserta referensi pengkabelan (wiring)
- Menambahkan API lengkap yang kompatibel dengan LiquidCrystal
- Menambahkan dukungan untuk cursor, blink, scroll, dan autoscroll
- Menambahkan makro perintah HD44780 yang sebelumnya belum ada
### Diubah (Changed)
- Refactor internal untuk penggunaan RAM dan flash yang lebih rendah
- Menyederhanakan alur inisialisasi tanpa mengubah perilaku lama (legacy behavior)
### Diperbaiki (Fixed)
- Peningkatan stabilitas inisialisasi I2C
- Urutan inisialisasi LCD 4-bit yang lebih andal
- Rilis stabil pertama dari IskakINO_LiquidCrystal_I2C

---
