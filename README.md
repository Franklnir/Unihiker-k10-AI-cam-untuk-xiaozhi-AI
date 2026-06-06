# Unihiker-k10-AI-cam-untuk-xiaozhi-AI
Unihiker k10 AI cam untuk xiaozhi AI
<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/e80c414c-22c8-4b92-a122-ca81beea2023" />

<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/f3282d00-0d52-424f-9757-df0c278902dc" />

<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/be25d21e-3900-498c-a855-b076f650a7da" />


<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/1a768f41-2aee-41d5-a47e-5fad4c3fa6ff" />

Panduan Penggunaan UNIHIKER K10 Xiaozhi
Dokumen ini menjelaskan cara menggunakan firmware Xiaozhi pada DFRobot UNIHIKER K10, mulai dari fungsi tombol, perintah suara Bahasa Indonesia, tools aktif, kamera, kartu SD, sensor, lampu RGB, Wi-Fi, hingga troubleshooting.
________________________________________
1. Ringkasan Fungsi
UNIHIKER K10 dengan firmware Xiaozhi dapat dikontrol melalui beberapa cara:
•	Tombol A dan B
•	Perintah suara Bahasa Indonesia
•	Kamera untuk mengambil foto dan menjelaskan isi gambar
•	Sensor suhu, kelembapan, cahaya, dan kemiringan
•	Lampu RGB
•	Kartu SD untuk melihat file, membaca file, menulis file, dan menyimpan foto
•	Reset atau konfigurasi ulang Wi-Fi
________________________________________
2. Fungsi Tombol
Tombol A
Aksi	Fungsi	Kegunaan
Klik sekali	Scroll chat ke atas	Melihat pesan lama
Klik dua kali	Listening / toggle chat	Mulai bicara ke Xiaozhi tanpa wake word
Tahan lama	Volume turun	Mengecilkan suara speaker
Klik tiga kali	Dinonaktifkan	Tidak melakukan apa-apa agar tidak salah reset Wi-Fi
Tombol B
Aksi	Fungsi	Kegunaan
Klik sekali	Scroll chat ke bawah	Melihat pesan terbaru
Klik dua kali	Tidak digunakan	Listening sudah dipindahkan ke tombol A double click
Klik tiga kali	Hapus Wi-Fi dan masuk konfigurasi Wi-Fi	Mengganti Wi-Fi, reset Wi-Fi, atau menghapus SSID
Tahan lama	Volume naik	Membesarkan suara speaker
Ring Klik tiga kali | Hapus Wi-Fi dan masuk konfigurasi Wi-Fi | Mengganti Wi-Fi, reset Wi-Fi, atau menghapus SSID |
kasan Cepat Tombol
Kebutuhan	Tombol
Melihat pesan lama	A klik sekali
Melihat pesan terbaru	B klik sekali
Mulai bicara	A klik dua kali
Ganti/reset Wi-Fi	B klik tiga kali
Volume turun	A tahan lama
Volume naik	B tahan lama
________________________________________
3. Cara Memberi Perintah Suara
Xiaozhi dapat menerima perintah menggunakan Bahasa Indonesia.
Contoh perintah:
•	“Cek status K10.”
•	“Baca semua sensor.”
•	“Nyalakan lampu merah.”
•	“Ambil foto dan jelaskan.”
•	“Simpan foto ke kartu SD.”
•	“Reset Wi-Fi.”
•	“Atur volume ke lima puluh.”
Untuk perintah reset atau ganti Wi-Fi melalui suara, Xiaozhi akan meminta konfirmasi terlebih dahulu.
Jawaban untuk melanjutkan:
•	“Iya.”
•	“Yes.”
Jawaban untuk membatalkan:
•	“Tidak.”
•	“Batal.”
________________________________________
4. Tools Aktif
Total tool aktif normal: 18 tools.
________________________________________
4.1 self.k10.get_status
Fungsi: mengecek status K10.
Contoh ucapan:
•	“Cek status K10.”
•	“Bagaimana status K10 sekarang?”
________________________________________
4.2 self.k10.get_sensors
Fungsi: membaca semua sensor K10, termasuk suhu, kelembapan, cahaya, dan kemiringan.
Contoh ucapan:
•	“Baca semua sensor.”
•	“Cek suhu, cahaya, dan kemiringan.”
•	“Bagaimana kondisi sensor sekarang?”
________________________________________
4.3 self.k10.set_rgb_led
Fungsi: mengatur lampu RGB K10.
Contoh ucapan:
•	“Nyalakan lampu merah.”
•	“Ubah lampu jadi biru.”
•	“Set lampu ke warna hijau.”
•	“Matikan lampu.”
________________________________________
4.4 self.light.set_rgb
Fungsi: mengatur lampu bawaan K10.
Contoh ucapan:
•	“Ubah lampu jadi putih.”
•	“Nyalakan lampu warna kuning.”
•	“Set lampu ke warna ungu.”
________________________________________
4.5 self.camera.get_status
Fungsi: mengecek status kamera.
Contoh ucapan:
•	“Cek kamera.”
•	“Apakah kamera aktif?”
•	“Status kamera bagaimana?”
________________________________________
4.6 self.camera.test_capture
Fungsi: mengetes apakah kamera bisa mengambil gambar.
Contoh ucapan:
•	“Tes kamera.”
•	“Coba ambil gambar test.”
•	“Apakah kamera bisa capture?”
________________________________________
4.7 self.camera.take_photo
Fungsi: mengambil foto, lalu meminta Xiaozhi menjelaskan isi foto tersebut.
Contoh ucapan:
•	“Ambil foto dan jelaskan.”
•	“Lihat ini dan jelaskan.”
•	“Foto ini, lalu beri tahu apa yang kamu lihat.”
•	“Ambil gambar dan jawab pertanyaan saya.”
Catatan:
Jika pengambilan foto atau upload foto gagal, layar tidak boleh stuck. Xiaozhi akan menampilkan notifikasi error.
________________________________________
4.8 self.camera.set_camera_flipped
Fungsi: membalik atau menormalkan arah kamera.
Contoh ucapan:
•	“Balik kamera.”
•	“Normalkan kamera.”
•	“Flip kamera.”
•	“Unflip kamera.”
________________________________________
4.9 self.k10.sd_mount
Fungsi: melakukan mount atau memasang kartu SD.
Contoh ucapan:
•	“Pasang kartu SD.”
•	“Mount kartu SD.”
•	“Deteksi kartu memori.”
________________________________________
4.10 self.k10.memory_card
Fungsi: mengecek kartu memori, melakukan mount kartu, atau menampilkan daftar file.
Contoh ucapan:
•	“Cek kartu memori.”
•	“Apakah kartu SD terbaca?”
•	“Tampilkan isi kartu memori.”
________________________________________
4.11 self.k10.sd_list_files
Fungsi: menampilkan daftar file di kartu SD.
Contoh ucapan:
•	“Tampilkan file di kartu SD.”
•	“List file di kartu memori.”
•	“Apa saja file di SD card?”
________________________________________
4.12 self.k10.sd_read_text
Fungsi: membaca file teks dari kartu SD.
Contoh ucapan:
•	“Baca file note.txt.”
•	“Baca catatan di kartu SD.”
•	“Buka file log.txt.”
________________________________________
4.13 self.k10.sd_write_text
Fungsi: menulis file teks ke kartu SD.
Contoh ucapan:
•	“Tulis catatan ke kartu SD.”
•	“Simpan teks ini ke note.txt.”
•	“Buat file catatan di kartu memori.”
________________________________________
4.14 self.k10.sd_save_camera_photo
Fungsi: mengambil foto dari kamera lalu menyimpannya ke kartu SD.
Contoh ucapan:
•	“Simpan foto ke kartu SD.”
•	“Ambil foto dan simpan ke memori.”
•	“Simpan gambar kamera ke SD card.”
________________________________________
4.15 self.wifi.reconfigure
Fungsi: reset Wi-Fi, hapus SSID, ganti Wi-Fi, atau masuk ke mode konfigurasi Wi-Fi melalui perintah suara.
Contoh ucapan:
•	“Reset Wi-Fi.”
•	“Ganti Wi-Fi.”
•	“Hapus SSID.”
•	“Masuk konfigurasi Wi-Fi.”
•	“Hapus Wi-Fi tersimpan.”
Alur konfirmasi:
1.	Ucapkan: “Reset Wi-Fi.”
2.	Xiaozhi akan meminta konfirmasi.
3.	Jawab: “Iya.”
4.	K10 akan masuk ke mode konfigurasi Wi-Fi.
Catatan:
Jika menggunakan tombol B klik tiga kali, K10 akan langsung menghapus Wi-Fi dan masuk ke konfigurasi Wi-Fi tanpa konfirmasi suara.
________________________________________
4.16 self.get_device_status
Fungsi: mengecek status umum perangkat, seperti volume, jaringan, dan kondisi device.
Contoh ucapan:
•	“Cek status perangkat.”
•	“Bagaimana kondisi perangkat?”
•	“Status device sekarang apa?”
________________________________________
4.17 self.audio_speaker.set_volume
Fungsi: mengatur volume speaker.
Contoh ucapan:
•	“Atur volume ke lima puluh.”
•	“Naikkan volume.”
•	“Turunkan volume.”
•	“Set volume ke seratus.”
•	“Mute speaker.”
________________________________________
4.18 self.screen.set_theme
Fungsi: mengubah tema layar.
Contoh ucapan:
•	“Ubah tema ke gelap.”
•	“Ubah tema ke terang.”
•	“Pakai mode gelap.”
•	“Pakai mode terang.”
________________________________________
5. Kamera
Kamera pada K10 dapat digunakan untuk:
•	Mengecek status kamera
•	Melakukan test capture
•	Mengambil foto lalu menjelaskan isi foto
•	Menyimpan foto ke kartu SD
•	Membalik arah kamera
Contoh penggunaan:
•	“Ambil foto dan jelaskan.”
•	“Apa yang kamu lihat?”
•	“Simpan foto ini ke kartu SD.”
•	“Balik kamera.”
Jika kamera gagal:
•	Pastikan firmware terbaru sudah diflash.
•	Coba ucapkan: “Tes kamera.”
•	Jika upload foto gagal, periksa koneksi Wi-Fi dan internet.
________________________________________
6. Kartu SD
Kartu SD dapat digunakan untuk:
•	Mount kartu SD
•	Mengecek status kartu SD
•	Menampilkan daftar file
•	Membaca file teks
•	Menulis file teks
•	Menyimpan foto kamera
Contoh penggunaan:
•	“Pasang kartu SD.”
•	“Cek kartu memori.”
•	“Tampilkan file di kartu SD.”
•	“Baca file note.txt.”
•	“Tulis catatan ke kartu SD.”
•	“Simpan foto ke kartu SD.”
Catatan:
•	Gunakan kartu SD dengan format FAT/FAT32.
•	Jika kartu SD tidak terbaca, cabut dan pasang ulang kartu.
•	Setelah itu, ucapkan: “Pasang kartu SD.”
________________________________________
7. Sensor
K10 dapat membaca beberapa sensor berikut:
•	Suhu
•	Kelembapan
•	Cahaya
•	Kemiringan / accelerometer
Contoh penggunaan:
•	“Baca semua sensor.”
•	“Berapa suhu sekarang?”
•	“Cek sensor cahaya.”
•	“Apakah perangkat miring?”
________________________________________
8. Lampu RGB
Lampu RGB dapat diatur menggunakan perintah suara.
Contoh penggunaan:
•	“Nyalakan lampu merah.”
•	“Ubah lampu jadi hijau.”
•	“Set lampu ke biru.”
•	“Matikan lampu.”
•	“Nyalakan lampu putih.”
________________________________________
9. Wi-Fi
Ada dua cara untuk masuk ke konfigurasi Wi-Fi.
________________________________________
9.1 Melalui Perintah Suara
Ucapkan salah satu perintah berikut:
•	“Reset Wi-Fi.”
•	“Ganti Wi-Fi.”
•	“Hapus SSID.”
•	“Masuk konfigurasi Wi-Fi.”
Xiaozhi akan meminta konfirmasi.
Jawab:
•	“Iya.”
Setelah itu, K10 akan masuk ke mode konfigurasi Wi-Fi.
________________________________________
9.2 Melalui Tombol
Tekan:
•	B klik tiga kali
K10 akan melakukan dua proses:
1.	Menghapus Wi-Fi/SSID yang tersimpan.
2.	Masuk ke mode konfigurasi Wi-Fi.
________________________________________
10. Layar dan Chat
Tampilan chat mendukung beberapa jenis pesan:
•	Pesan user
•	Pesan assistant
•	Pesan sistem
•	Preview foto
•	Notifikasi error
Kontrol layar:
Aksi	Fungsi
A klik sekali	Scroll ke atas
B klik sekali	Scroll ke bawah
Perintah suara “Ubah tema ke gelap”	Mengaktifkan mode gelap
Perintah suara “Ubah tema ke terang”	Mengaktifkan mode terang
________________________________________
11. Volume
Volume dapat dikontrol melalui perintah suara atau tombol.
Melalui Perintah Suara
Contoh:
•	“Atur volume ke lima puluh.”
•	“Naikkan volume.”
•	“Turunkan volume.”
•	“Mute speaker.”
Melalui Tombol
Tombol	Fungsi
A tahan lama	Volume turun
B tahan lama	Volume naik
________________________________________
12. Troubleshooting
12.1 Layar Terlihat Stuck
Kemungkinan proses kamera atau upload sedang gagal. Firmware sudah diperbaiki agar proses kamera berjalan di background dan tidak menahan layar.
Solusi:
•	Tunggu sampai notifikasi error muncul.
•	Coba lagi saat Wi-Fi stabil.
•	Ucapkan: “Tes kamera.”
________________________________________
12.2 Kamera Gagal Mengambil Foto
Solusi:
•	Ucapkan: “Cek kamera.”
•	Ucapkan: “Tes kamera.”
•	Restart perangkat jika kamera tetap tidak ready.
________________________________________
12.3 Foto Gagal Dikirim
Solusi:
•	Pastikan Wi-Fi tersambung.
•	Pastikan internet aktif.
•	Coba ulangi perintah: “Ambil foto dan jelaskan.”
________________________________________
12.4 Kartu SD Tidak Terbaca
Solusi:
•	Pastikan kartu SD menggunakan format FAT/FAT32.
•	Cabut dan pasang ulang kartu SD.
•	Ucapkan: “Pasang kartu SD.”
•	Ucapkan: “Cek kartu memori.”
________________________________________
12.5 Wi-Fi Ingin Diganti
Gunakan salah satu cara berikut:
•	Ucapkan: “Reset Wi-Fi”, lalu jawab “Iya”.
•	Tekan B klik tiga kali.
________________________________________
12.6 COM Port Tidak Terdeteksi Saat Flash
Solusi:
•	Cabut dan pasang ulang kabel USB.
•	Pastikan kabel USB mendukung transfer data, bukan hanya charging.
•	Cek Device Manager.
•	Pastikan port yang dipilih benar, misalnya COM3.
________________________________________
13. Catatan Penting
•	Perintah suara dapat menggunakan Bahasa Indonesia.
•	Tool aktif normal saat ini berjumlah 18 tools.
•	Total tool termasuk user-only/internal berjumlah 25 tools.
•	Batas sistem Xiaozhi adalah 32 tools.
•	Beberapa tool sengaja dimatikan agar kamera, SD card, sensor, lampu, dan Wi-Fi tetap aman.
•	A triple click dinonaktifkan agar tidak terjadi reset Wi-Fi secara tidak sengaja.
•	B triple click digunakan khusus untuk reset atau ganti Wi-Fi.
________________________________________
14. Ringkasan Super Singkat
Kebutuhan	Cara
Mulai bicara	A klik dua kali
Scroll chat ke atas	A klik sekali
Scroll chat ke bawah	B klik sekali
Reset/ganti Wi-Fi	B klik tiga kali
Volume turun	A tahan lama
Volume naik	B tahan lama
Cek sensor	Ucapkan “Baca semua sensor”
Ambil foto	Ucapkan “Ambil foto dan jelaskan”
Simpan foto ke SD	Ucapkan “Simpan foto ke kartu SD”
Ubah tema	Ucapkan “Ubah tema ke gelap/terang”

