/*
  IskakINO_LiquidCrystal_I2C - v1.1.0 Feature Demo
  Mendemokan 8 fitur baru:
  1. Non-blocking typewriter
  2. Overload const char* (hemat RAM)
  3. Backlight auto-timeout
  4. Auto horizontal scroll text
  5. Progress bar built-in
  6. Multi-LCD safe init (lihat komentar di bawah)
  7. printFormatted() printf-style
  12. Preset ikon custom character

  Wiring: SDA/SCL default board kamu (auto-detect alamat I2C).
*/

#include <IskakINO_LiquidCrystal_I2C.h>
#include <IskakINO_LCD_Icons.h>

LiquidCrystal_I2C lcd(16, 2);

unsigned long lastDemoSwitch = 0;
uint8_t demoStage = 0;

void setup() {
  lcd.begin();

  // FITUR #3: backlight otomatis mati setelah 15 detik idle
  lcd.setBacklightTimeout(15000);

  // FITUR #12: daftarkan beberapa ikon (hindari slot 7, dipakai progress bar)
  lcd.createChar(0, ICON_THERMOMETER);
  lcd.createChar(1, ICON_WIFI);

  // FITUR #1: mulai animasi typewriter non-blocking
  lcd.clear();
  lcd.typewriterStart("Hello IskakINO!", 0, 80);
}

void loop() {
  // WAJIB dipanggil setiap loop() untuk fitur non-blocking (#1, #3, #4)
  lcd.update();

  // Ganti demo tiap 4 detik setelah typewriter pertama selesai
  if (!lcd.isTypewriterActive() && !lcd.isScrollActive()
      && millis() - lastDemoSwitch > 4000) {

    lastDemoSwitch = millis();
    demoStage = (demoStage + 1) % 4;
    lcd.clear();

    switch (demoStage) {
      case 0:
        // FITUR #2: overload const char*, hemat RAM dibanding String
        lcd.printCenter("Centered Text", 0);
        break;

      case 1:
        // FITUR #7: printf-style
        lcd.setCursor(0, 0);
        lcd.write((uint8_t)0); // ikon termometer
        lcd.printFormatted(" Suhu: %d", 27);
        lcd.write((uint8_t)0xDF); // simbol derajat bawaan HD44780 (opsional)
        lcd.print("C");
        break;

      case 2:
        // FITUR #5: progress bar
        for (int p = 0; p <= 100; p += 20) {
          lcd.drawProgressBar(p, 1);
          delay(150);
        }
        break;

      case 3:
        // FITUR #4: auto scroll untuk teks panjang
        lcd.scrollTextStart("Ini adalah teks panjang yang akan berjalan otomatis...", 1, 250);
        break;
    }
  }
}

/*
  CATATAN FITUR #6 (Multi-LCD):
  Kalau kamu pakai 2 LCD sekaligus di bus I2C yang sama (alamat beda),
  cukup buat 2 instance seperti biasa — Wire.begin() otomatis cuma
  dipanggil sekali di belakang layar:

    LiquidCrystal_I2C lcd1(16, 2);
    LiquidCrystal_I2C lcd2(16, 2);

    void setup() {
      lcd1.begin();  // Wire.begin() dipanggil di sini
      lcd2.begin();  // Wire.begin() TIDAK dipanggil ulang (aman)
    }
*/
