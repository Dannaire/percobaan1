// --- DATA KALIBRASI 3 TITIK (Multi-Point Calibration) ---
//coba ubah kodingan
// Nilai pembacaan lama (sebelum kalibrasi) vs Nilai pH standar
const float PH_READ_ASAM   = 5.40;   // Pembacaan lama saat di larutan Asam
const float PH_TRUE_ASAM   = 4.01;   // Nilai sebenarnya (Buffer 4.01)

const float PH_READ_NETRAL = 8.50;   // Pembacaan lama saat di larutan Netral
const float PH_TRUE_NETRAL = 6.80;   // Nilai sebenarnya (Buffer 6.80)

const float PH_READ_BASA   = 10.75;  // Pembacaan lama saat di larutan Basa
const float PH_TRUE_BASA   = 9.18;   // Nilai sebenarnya (Buffer 9.18)

const float PH_OFFSET      = -0.12;  // Koreksi offset (karena rata-rata kelebihan 0.1)

float calibration_value = 21.34 + 1.5;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

// Tentukan pin analog untuk sensor pH
#define PH_SENSOR_PIN 34   // GPIO34 untuk input sensor pH pada ESP32

void setup() {
  Serial.begin(115200);    // Baudrate ESP32
  pinMode(PH_SENSOR_PIN, INPUT);
  Serial.println("pH Sensor Ready (3-Point Calibrated)...");
}

void loop() {
  // Baca data analog sebanyak 10 kali
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(30);
  }

  // Urutkan data (bubble sort) untuk membuang nilai ekstrem
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // Ambil rata-rata dari 6 sampel di tengah (abaikan 2 terendah dan 2 tertinggi)
  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }
  float adc_avg = (float)avgval / 6.0;

  // Konversi ke tegangan ESP32 (3.3V dengan resolusi 12-bit / 4095)
  float volt = adc_avg * 3.3 / 4095.0;
  
  // 1. Hitung nilai pH mentah dengan rumus dasar terlebih dahulu
  float ph_raw = -5.70 * volt + calibration_value;

  // 2. Kalibrasi 3 Titik (Piecewise Linear Interpolation)
  if (ph_raw <= PH_READ_NETRAL) {
    // Rentang Asam ke Netral (<= 8.50)
    float slope = (PH_TRUE_NETRAL - PH_TRUE_ASAM) / (PH_READ_NETRAL - PH_READ_ASAM);
    ph_act = PH_TRUE_ASAM + (ph_raw - PH_READ_ASAM) * slope;
  } else {
    // Rentang Netral ke Basa (> 8.50)
    float slope = (PH_TRUE_BASA - PH_TRUE_NETRAL) / (PH_READ_BASA - PH_READ_NETRAL);
    ph_act = PH_TRUE_NETRAL + (ph_raw - PH_READ_NETRAL) * slope;
  }

  // 3. Tambahkan koreksi offset (-0.10)
  ph_act += PH_OFFSET;

  // Tampilkan Nilai ADC Raw, Tegangan, dan pH yang sudah dikalibrasi di Serial Monitor
  Serial.print("Nilai ADC: ");
  Serial.print(adc_avg, 0);
  Serial.print("\t | \tTegangan: ");
  Serial.print(volt, 3);
  Serial.print(" V\t | \tpH Value: ");
  Serial.println(ph_act, 2);

  delay(1000); // Delay untuk kestabilan pembacaan
}
