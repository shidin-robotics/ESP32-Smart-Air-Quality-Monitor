/*
 * Project: IskakINO_LiquidCrystal_I2C
 * Folder: examples/05_ProgressBar
 * Description: Dynamic loading progress bar.
 * Author: Iskak Fatoni
 *
 * [UPDATED v1.1.0]
 * Sebelumnya contoh ini bikin custom character block sendiri secara manual.
 * Sekarang cukup pakai lcd.drawProgressBar(percent, row) yang sudah built-in
 * di library (otomatis pakai custom character slot 7).
 */

#include <IskakINO_LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(16, 2);

void setup() {
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Loading:");
}

void loop() {
  for (int percent = 0; percent <= 100; percent += 5) {
    // Tampilkan progress bar di baris ke-2 (built-in, cuma 1 baris kode)
    lcd.drawProgressBar(percent, 1);

    // Tampilkan persentase di baris pertama
    lcd.setCursor(12, 0);
    if (percent < 100) lcd.print(" "); // Padding spasi
    lcd.print(percent);
    lcd.print("%");

    delay(150); // Kecepatan loading
  }

  delay(2000); // Berhenti sejenak sebelum mengulang
}
