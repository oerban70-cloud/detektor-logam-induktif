# Detektor Logam dengan Sensor Induktif + Buzzer — ESP32

Paku kecil jatuh di rumput, mencarinya bisa seharian. Alat kecil ini
menemukan logam dari jarak 4 mm, lalu buzzernya langsung berbunyi dan LED
menyala — kamu tahu persis di mana logam itu bersembunyi.

Sensor induktif LJ12A3-4-Z/BX bekerja dengan kumparan yang medan magnetnya
berubah saat logam mendekat. Keluaran sensornya open-collector NPN: saat
logam terdeteksi, output menarik ke GND (LOW); saat tidak ada logam, output
mengambang dan ditarik HIGH oleh pull-up internal ESP32. ESP32 membaca
perubahan itu, lalu menyalakan buzzer aktif 5 V lewat transistor 2N2222
dan LED hijau sebagai indikator visual.

Video penjelasan (60 detik):
link menyusul setelah upload

## Komponen

| Bagian | Keterangan |
|---|---|
| ESP32 DevKit | Otak alat |
| Sensor induktif LJ12A3-4-Z/BX | NPN NO, catu 6-36 V DC, jarak deteksi 4 mm |
| Buzzer aktif 5 V | Bunyi saat logam terdeteksi |
| Transistor 2N2222 | Saklar NPN untuk buzzer |
| Resistor 1 kΩ | Basis transistor |
| LED hijau + resistor 220 Ω | Indikator visual saat logam terdeteksi |
| Adaptor 12 V DC | Catu sensor induktif |
| Adaptor USB 5 V | Catu ESP32 |

## Wiring

| Dari | Ke |
|---|---|
| Sensor coklat (+12 V) | Adaptor 12 V (+) |
| Sensor hitam (GND) | GND gabungan (adaptor 12 V + ESP32 GND) |
| Sensor biru (OUT) | ESP32 GPIO27 (INPUT_PULLUP) |
| ESP32 GPIO26 | Resistor 1 kΩ → basis 2N2222 |
| 2N2222 kolektor | Buzzer (-) |
| Buzzer (+) | 5 V (VIN ESP32) |
| 2N2222 emitor | GND |
| ESP32 GPIO25 | Resistor 220 Ω → anoda LED hijau |
| Katoda LED | GND |

Perhatian: GPIO34-39 tidak punya pull-up internal, jadi sensor WAJIB
masuk ke GPIO yang punya pull-up (dipakai GPIO27). Sensor butuh catu
6-36 V — jangan dicatu dari pin 3V3 ESP32, pakai adaptor 12 V sendiri
dan gabungkan GND-nya dengan GND ESP32.

## Cara kerja

1. Kumparan di dalam sensor LJ12A3 menghasilkan medan magnet kecil di
   muka deteksinya.
2. Logam yang mendekat sampai 4 mm mengubah medan itu, dan sensor
   menarik outputnya ke GND (LOW).
3. ESP32 membaca GPIO27 dengan debounce 50 ms, lalu menyalakan buzzer
   pola bip-bip (nyala 150 ms, mati 150 ms, non-blocking) dan LED hijau.
4. Saat logam menjauh, output sensor kembali HIGH, buzzer diam, LED mati.
5. Status perubahan dicetak ke Serial 115200 ("LOGAM TERDETEKSI" /
   "Tidak ada logam").

## Konfigurasi

Semua pengaturan ada di bagian atas sketch
(`DetektorLogamInduktif/DetektorLogamInduktif.ino`):

| Konstanta | Nilai | Keterangan |
|---|---|---|
| `PIN_SENSOR` | 27 | Input sensor (INPUT_PULLUP) |
| `PIN_BUZZER` | 26 | Output ke basis transistor |
| `PIN_LED` | 25 | Output ke LED hijau |
| `WAKTU_DEBOUNCE_MS` | 50 | Debounce sensor |
| `BEEP_NYALA_MS` | 150 | Durasi bunyi tiap bip |
| `BEEP_MATI_MS` | 150 | Jeda antar bip |

## Hasil compile (terverifikasi)

- Tool: arduino-cli 1.6.0, ESP32 core 4.0.0, board `esp32:esp32:esp32`
- Ukuran sketch (flash): 291.817 byte (22% dari 1.310.720 byte)
- RAM (variabel global): 23.232 byte (7% dari 327.680 byte)
- Tanpa library eksternal — hanya Arduino core ESP32.

## Skema

Lihat `schematic.png` — gaya Fritzing, wiring ortogonal, warna kabel per
net (merah = daya, hitam = GND), transistor digambar dengan simbol NPN
asli (panah emitor keluar dari basis), resistor basis dan LED digambar
zigzag di jalurnya.
