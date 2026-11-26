// Detektor Logam dengan Sensor Induktif + Buzzer
// Board: ESP32 (esp32:esp32:esp32)
// Sensor : LJ12A3-4-Z/BX, NPN NO, catu 6-36V DC, jarak deteksi 4 mm
//          Output open-collector: LOW saat logam terdeteksi,
//          mengambang (HIGH lewat pull-up) saat tidak ada logam.
// Buzzer : buzzer aktif 5V lewat transistor NPN 2N2222
//          (basis via resistor 1k, kolektor ke buzzer (-),
//          buzzer (+) ke 5V, emitor ke GND).
// LED    : LED hijau, anoda via resistor 220 ohm, katoda ke GND.
//
// Perilaku:
//   - Logam terdeteksi  -> LED menyala, buzzer bip-bip
//                          (nyala 150 ms, mati 150 ms, non-blocking).
//   - Tidak ada logam   -> LED mati, buzzer diam.
//   - Serial 115200 mencetak status hanya saat berubah.

// ---------------------------------------------------------------------------
// Konfigurasi pin
// ---------------------------------------------------------------------------
const uint8_t PIN_SENSOR = 27;  // GPIO27, INPUT_PULLUP (punya pull-up internal)
const uint8_t PIN_BUZZER = 26;  // GPIO26 -> basis 2N2222 via resistor 1k
const uint8_t PIN_LED    = 25;  // GPIO25 -> anoda LED hijau via resistor 220 ohm

// ---------------------------------------------------------------------------
// Konfigurasi waktu (milidetik)
// ---------------------------------------------------------------------------
const unsigned long WAKTU_DEBOUNCE_MS = 50;   // Waktu stabil sensor sebelum diterima
const unsigned long BEEP_NYALA_MS     = 150;  // Durasi buzzer menyala tiap bip
const unsigned long BEEP_MATI_MS      = 150;  // Durasi jeda antar bip

// ---------------------------------------------------------------------------
// State internal
// ---------------------------------------------------------------------------
bool     logamTerdeteksi   = false;  // Status stabil sensor (true = ada logam)
bool     buzzerMenyala     = false;  // State buzzer saat ini dalam pola bip-bip
unsigned long waktuBuzzerTerakhir = 0;  // Terakhir kali buzzer berganti state

// State debounce sensor (pembacaan mentah bisa bergetar saat transisi)
int            bacaanSensorTerakhir = HIGH;  // Pembacaan mentah terakhir
int            bacaanSensorStabil   = HIGH;  // Pembacaan mentah yang sudah stabil
unsigned long  waktuPerubahanSensor = 0;     // Terakhir kali bacaan mentah berubah

// ---------------------------------------------------------------------------
// bacaSensor: membaca sensor induktif dengan debounce.
// Mengembalikan true jika logam terdeteksi (output sensor LOW yang stabil),
// false jika tidak ada logam (output sensor HIGH yang stabil).
// ---------------------------------------------------------------------------
bool bacaSensor() {
  int bacaan = digitalRead(PIN_SENSOR);

  // Catat waktu setiap kali bacaan mentah berubah.
  if (bacaan != bacaanSensorTerakhir) {
    waktuPerubahanSensor = millis();
    bacaanSensorTerakhir = bacaan;
  }

  // Terima bacaan baru hanya setelah stabil selama WAKTU_DEBOUNCE_MS.
  if ((millis() - waktuPerubahanSensor) > WAKTU_DEBOUNCE_MS) {
    if (bacaan != bacaanSensorStabil) {
      bacaanSensorStabil = bacaan;
    }
  }

  // Sensor NPN NO: LOW = logam terdeteksi, HIGH = tidak ada logam.
  return (bacaanSensorStabil == LOW);
}

// ---------------------------------------------------------------------------
// aturLed: menyalakan/mematikan LED indikator hijau.
// ---------------------------------------------------------------------------
void aturLed(bool nyala) {
  digitalWrite(PIN_LED, nyala ? HIGH : LOW);
}

// ---------------------------------------------------------------------------
// aturBuzzer: menyalakan/mematikan buzzer (lewat transistor 2N2222).
// ---------------------------------------------------------------------------
void aturBuzzer(bool nyala) {
  digitalWrite(PIN_BUZZER, nyala ? HIGH : LOW);
}

// ---------------------------------------------------------------------------
// updateBuzzer: mengatur pola bip-bip (non-blocking, pakai millis).
// Dipanggil setiap loop; hanya aktif saat logam terdeteksi.
// Saat tidak ada logam, buzzer dimatikan dan state pola di-reset.
// ---------------------------------------------------------------------------
void updateBuzzer(bool terdeteksi) {
  if (!terdeteksi) {
    // Tidak ada logam: buzzer diam, reset state pola.
    if (buzzerMenyala) {
      buzzerMenyala = false;
      aturBuzzer(false);
    }
    waktuBuzzerTerakhir = millis();
    return;
  }

  // Ada logam: toggling buzzer tiap BEEP_NYALA_MS / BEEP_MATI_MS.
  unsigned long sekarang = millis();
  unsigned long durasiStateSaatIni = buzzerMenyala ? BEEP_NYALA_MS : BEEP_MATI_MS;

  if ((sekarang - waktuBuzzerTerakhir) >= durasiStateSaatIni) {
    buzzerMenyala = !buzzerMenyala;
    aturBuzzer(buzzerMenyala);
    waktuBuzzerTerakhir = sekarang;
  }
}

// ---------------------------------------------------------------------------
// cetakStatus: mencetak status ke Serial hanya saat status berubah.
// ---------------------------------------------------------------------------
void cetakStatus(bool terdeteksi) {
  if (terdeteksi) {
    Serial.println("LOGAM TERDETEKSI");
  } else {
    Serial.println("Tidak ada logam");
  }
}

void setup() {
  Serial.begin(115200);

  // Sensor: INPUT_PULLUP karena output open-collector mengambang saat
  // tidak ada logam. JANGAN pakai GPIO34-39: tidak punya pull-up internal.
  pinMode(PIN_SENSOR, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Kondisi awal: LED mati, buzzer diam.
  aturLed(false);
  aturBuzzer(false);

  // Inisialisasi state debounce dari kondisi sensor saat ini.
  bacaanSensorTerakhir = digitalRead(PIN_SENSOR);
  bacaanSensorStabil   = bacaanSensorTerakhir;
  waktuPerubahanSensor = millis();
  logamTerdeteksi      = (bacaanSensorStabil == LOW);

  Serial.println("Detektor Logam Induktif siap.");
  cetakStatus(logamTerdeteksi);
}

void loop() {
  bool terdeteksi = bacaSensor();

  // Tangani perubahan status: update LED dan cetak ke Serial.
  if (terdeteksi != logamTerdeteksi) {
    logamTerdeteksi = terdeteksi;
    aturLed(logamTerdeteksi);
    cetakStatus(logamTerdeteksi);
  }

  // Pola bip-bip buzzer (non-blocking).
  updateBuzzer(logamTerdeteksi);
}
