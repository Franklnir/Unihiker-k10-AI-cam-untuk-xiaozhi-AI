# Panduan Penggunaan UNIHIKER K10 Xiaozhi

Dokumen ini menjelaskan cara menggunakan firmware Xiaozhi untuk DFRobot UNIHIKER K10, termasuk fungsi tombol, perintah suara Bahasa Indonesia, tools aktif, kamera, kartu SD, lampu, sensor, Wi-Fi, dan troubleshooting.

## 1. Ringkasan Fungsi

K10 bisa dikontrol lewat:

- Tombol A dan B
- Perintah suara Bahasa Indonesia
- Kamera untuk ambil foto dan tanya isi foto
- Sensor suhu, kelembapan, cahaya, dan kemiringan
- Lampu RGB
- Kartu SD untuk list file, baca file, tulis file, dan simpan foto
- Wi-Fi reset atau masuk konfigurasi Wi-Fi

## 2. Fungsi Tombol

### Tombol A

| Aksi | Fungsi | Kegunaan |
| --- | --- | --- |
| A klik sekali | Scroll chat ke atas | Melihat pesan lama |
| A klik dua kali | Listening / toggle chat | Mulai bicara ke Xiaozhi tanpa wake word |
| A tahan lama | Volume turun | Mengecilkan suara speaker |
| A klik tiga kali | Dimatikan | Tidak melakukan apa-apa supaya tidak salah reset Wi-Fi |

### Tombol B

| Aksi | Fungsi | Kegunaan |
| --- | --- | --- |
| B klik sekali | Scroll chat ke bawah | Melihat pesan terbaru |
| B klik dua kali | Tidak dipakai | Listening sudah dipindah ke A double click |
| B klik tiga kali | Hapus Wi-Fi + masuk konfigurasi Wi-Fi | Ganti Wi-Fi, reset Wi-Fi, atau hapus SSID |
| B tahan lama | Volume naik | Membesarkan suara speaker |

### Ringkasan Cepat Tombol

- Lihat pesan lama: **A sekali**
- Lihat pesan terbaru: **B sekali**
- Mulai bicara: **A dua kali**
- Ganti/reset Wi-Fi: **B tiga kali**
- Volume turun: **A tahan**
- Volume naik: **B tahan**

## 3. Cara Memberi Perintah Suara

Kamu bisa pakai Bahasa Indonesia. Contoh:

- "Cek status K10."
- "Baca semua sensor."
- "Nyalakan lampu merah."
- "Ambil foto dan jelaskan."
- "Simpan foto ke kartu SD."
- "Reset Wi-Fi."
- "Atur volume ke lima puluh."

Untuk perintah reset/ganti Wi-Fi lewat suara, Xiaozhi akan meminta konfirmasi. Jawab:

- "Iya."
- "Yes."

Jika tidak mau lanjut, jawab:

- "Tidak."
- "Batal."

## 4. Tools Aktif Dan Cara Mengucapkannya

Total tool aktif normal: **18 tools**.

### 1. `self.k10.get_status`

Fungsi: cek status K10.

Contoh ucapan:

- "Cek status K10."
- "Bagaimana status K10 sekarang?"

### 2. `self.k10.get_sensors`

Fungsi: baca semua sensor K10, termasuk suhu, kelembapan, cahaya, dan kemiringan.

Contoh ucapan:

- "Baca semua sensor."
- "Cek suhu, cahaya, dan kemiringan."
- "Bagaimana kondisi sensor sekarang?"

### 3. `self.k10.set_rgb_led`

Fungsi: mengatur lampu RGB K10.

Contoh ucapan:

- "Nyalakan lampu merah."
- "Ubah lampu jadi biru."
- "Set lampu ke warna hijau."
- "Matikan lampu."

### 4. `self.light.set_rgb`

Fungsi: mengatur lampu bawaan K10.

Contoh ucapan:

- "Ubah lampu jadi putih."
- "Nyalakan lampu warna kuning."
- "Set lampu ke warna ungu."

### 5. `self.camera.get_status`

Fungsi: cek status kamera.

Contoh ucapan:

- "Cek kamera."
- "Apakah kamera aktif?"
- "Status kamera bagaimana?"

### 6. `self.camera.test_capture`

Fungsi: tes apakah kamera bisa mengambil gambar.

Contoh ucapan:

- "Tes kamera."
- "Coba ambil gambar test."
- "Apakah kamera bisa capture?"

### 7. `self.camera.take_photo`

Fungsi: ambil foto lalu minta Xiaozhi menjelaskan isi foto.

Contoh ucapan:

- "Ambil foto dan jelaskan."
- "Lihat ini dan jelaskan."
- "Foto ini, lalu beri tahu apa yang kamu lihat."
- "Ambil gambar dan jawab pertanyaan saya."

Catatan: jika ambil foto atau upload foto gagal, layar tidak boleh stuck. Xiaozhi akan menampilkan notifikasi error.

### 8. `self.camera.set_camera_flipped`

Fungsi: membalik atau menormalkan arah kamera.

Contoh ucapan:

- "Balik kamera."
- "Normalkan kamera."
- "Flip kamera."
- "Unflip kamera."

### 9. `self.k10.sd_mount`

Fungsi: mount atau pasang kartu SD.

Contoh ucapan:

- "Pasang kartu SD."
- "Mount kartu SD."
- "Deteksi kartu memori."

### 10. `self.k10.memory_card`

Fungsi: cek kartu memori, mount kartu, atau list file.

Contoh ucapan:

- "Cek kartu memori."
- "Apakah kartu SD terbaca?"
- "Tampilkan isi kartu memori."

### 11. `self.k10.sd_list_files`

Fungsi: menampilkan daftar file di kartu SD.

Contoh ucapan:

- "Tampilkan file di kartu SD."
- "List file di kartu memori."
- "Apa saja file di SD card?"

### 12. `self.k10.sd_read_text`

Fungsi: membaca file teks dari kartu SD.

Contoh ucapan:

- "Baca file note.txt."
- "Baca catatan di kartu SD."
- "Buka file log.txt."

### 13. `self.k10.sd_write_text`

Fungsi: menulis file teks ke kartu SD.

Contoh ucapan:

- "Tulis catatan ke kartu SD."
- "Simpan teks ini ke note.txt."
- "Buat file catatan di kartu memori."

### 14. `self.k10.sd_save_camera_photo`

Fungsi: ambil foto dari kamera lalu simpan ke kartu SD.

Contoh ucapan:

- "Simpan foto ke kartu SD."
- "Ambil foto dan simpan ke memori."
- "Simpan gambar kamera ke SD card."

### 15. `self.wifi.reconfigure`

Fungsi: reset Wi-Fi, hapus SSID, ganti Wi-Fi, atau masuk konfigurasi Wi-Fi lewat suara.

Contoh ucapan:

- "Reset Wi-Fi."
- "Ganti Wi-Fi."
- "Hapus SSID."
- "Masuk konfigurasi Wi-Fi."
- "Hapus Wi-Fi tersimpan."

Alur konfirmasi:

1. Kamu ucapkan: "Reset Wi-Fi."
2. Xiaozhi akan meminta konfirmasi.
3. Kamu jawab: "Iya."
4. K10 akan masuk mode konfigurasi Wi-Fi.

Catatan: kalau memakai tombol **B klik tiga kali**, K10 langsung hapus Wi-Fi dan masuk konfigurasi Wi-Fi tanpa konfirmasi suara.

### 16. `self.get_device_status`

Fungsi: cek status umum perangkat, seperti volume, jaringan, dan kondisi device.

Contoh ucapan:

- "Cek status perangkat."
- "Bagaimana kondisi perangkat?"
- "Status device sekarang apa?"

### 17. `self.audio_speaker.set_volume`

Fungsi: mengatur volume speaker.

Contoh ucapan:

- "Atur volume ke lima puluh."
- "Naikkan volume."
- "Turunkan volume."
- "Set volume ke seratus."
- "Mute speaker."

### 18. `self.screen.set_theme`

Fungsi: mengubah tema layar.

Contoh ucapan:

- "Ubah tema ke gelap."
- "Ubah tema ke terang."
- "Pakai mode gelap."
- "Pakai mode terang."

## 5. Kamera

Kamera bisa dipakai untuk:

- Cek status kamera
- Tes capture
- Ambil foto lalu tanya isi foto
- Simpan foto ke kartu SD
- Balik arah kamera

Contoh penggunaan:

- "Ambil foto dan jelaskan."
- "Apa yang kamu lihat?"
- "Simpan foto ini ke kartu SD."
- "Balik kamera."

Jika kamera gagal:

- Pastikan firmware terbaru sudah diflash.
- Coba ucapkan "Tes kamera."
- Jika upload foto gagal, periksa Wi-Fi dan koneksi internet.

## 6. Kartu SD

Kartu SD bisa dipakai untuk:

- Mount kartu SD
- Cek status kartu SD
- List file
- Baca file teks
- Tulis file teks
- Simpan foto kamera

Contoh penggunaan:

- "Pasang kartu SD."
- "Cek kartu memori."
- "Tampilkan file di kartu SD."
- "Baca file note.txt."
- "Tulis catatan ke kartu SD."
- "Simpan foto ke kartu SD."

Catatan:

- Gunakan kartu SD format FAT/FAT32.
- Jika tidak terbaca, cabut dan pasang ulang kartu, lalu ucapkan "Pasang kartu SD."

## 7. Sensor

K10 dapat membaca:

- Suhu
- Kelembapan
- Cahaya
- Kemiringan / accelerometer

Contoh penggunaan:

- "Baca semua sensor."
- "Berapa suhu sekarang?"
- "Cek sensor cahaya."
- "Apakah perangkat miring?"

## 8. Lampu RGB

Lampu bisa diatur dengan warna.

Contoh penggunaan:

- "Nyalakan lampu merah."
- "Ubah lampu jadi hijau."
- "Set lampu ke biru."
- "Matikan lampu."
- "Nyalakan lampu putih."

## 9. Wi-Fi

Ada dua cara masuk konfigurasi Wi-Fi.

### Lewat Suara

Ucapkan:

- "Reset Wi-Fi."
- "Ganti Wi-Fi."
- "Hapus SSID."
- "Masuk konfigurasi Wi-Fi."

Xiaozhi akan meminta konfirmasi. Jawab:

- "Iya."

### Lewat Tombol

Tekan:

- **B klik tiga kali**

K10 akan:

1. Menghapus Wi-Fi/SSID tersimpan.
2. Masuk konfigurasi Wi-Fi.

## 10. Layar Dan Chat

Tampilan chat mendukung dua arah:

- Pesan user
- Pesan assistant
- Pesan sistem
- Preview foto
- Notifikasi error

Kontrol layar:

- **A klik sekali**: scroll ke atas
- **B klik sekali**: scroll ke bawah
- Perintah suara: "Ubah tema ke gelap" atau "Ubah tema ke terang"

## 11. Volume

Kontrol suara:

- "Atur volume ke lima puluh."
- "Naikkan volume."
- "Turunkan volume."

Kontrol tombol:

- **A tahan lama**: volume turun
- **B tahan lama**: volume naik

## 12. Troubleshooting

### Layar seperti stuck

Kemungkinan proses kamera atau upload sedang gagal. Firmware sudah diperbaiki agar proses kamera berjalan di background dan tidak menahan layar.

Solusi:

- Tunggu sampai notifikasi error muncul.
- Coba lagi saat Wi-Fi stabil.
- Coba "Tes kamera."

### Kamera gagal ambil foto

Solusi:

- Ucapkan "Cek kamera."
- Ucapkan "Tes kamera."
- Restart perangkat jika kamera tetap tidak ready.

### Foto gagal dikirim

Solusi:

- Pastikan Wi-Fi tersambung.
- Pastikan internet aktif.
- Coba ulang perintah "Ambil foto dan jelaskan."

### Kartu SD tidak terbaca

Solusi:

- Pastikan kartu SD format FAT/FAT32.
- Cabut pasang kartu SD.
- Ucapkan "Pasang kartu SD."
- Ucapkan "Cek kartu memori."

### Wi-Fi ingin diganti

Pakai salah satu:

- Ucapkan "Reset Wi-Fi", lalu jawab "Iya".
- Atau tekan **B klik tiga kali**.

### COM port tidak terdeteksi saat flash

Solusi:

- Cabut pasang kabel USB.
- Pastikan kabel USB mendukung data, bukan hanya charging.
- Cek Device Manager.
- Pastikan port benar, misalnya `COM3`.

## 13. Catatan Penting

- Perintah suara bisa memakai Bahasa Indonesia.
- Tool aktif normal saat ini ada **18 tools**.
- Total termasuk user-only/internal ada **25 tools**.
- Batas sistem Xiaozhi adalah 32 tools, jadi beberapa tool sengaja dimatikan agar kamera, SD card, sensor, lampu, dan Wi-Fi tetap aman.
- **A triple click dimatikan** supaya tidak salah reset Wi-Fi.
- **B triple click** dipakai khusus untuk reset/ganti Wi-Fi.

