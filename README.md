Pada konsepnya, program ini saya rancang dengan konsep ketika suatu objek bundar berwarna ditunjukan pada webcam, maka program tersebut akan mendeteksi objek tersebut dan menganalisa warnanya. Setelah warna tersebut dianalisa maka program akan mengeluarkan output sebuah poin ketika objek berwarna tersebut sudah terdeteksi dalam jangakauan 2 detik.

Cukup banyak permasalahan yang dihadapain dalam pengerjaan soal ini mulai dari webcam tidak bisa diakses, saya atasi dengan beberapa command pada terminal.
Lalu, terminal yang crash ketika dibuka lebih dari satu. Deteksi yang kadang error, dalam contoh yang saya tunjukan tutup botol a yang terdeteksi b, saya selesaikan dengan menaikan parameternya.
Lalu saya juga kesulitan mengubah file yolo26.pt menjadi dalam benntuk onnx.
Kemudian pada konsep awalnya saya menggunakan train dataset untuk mendapatkan hasil yang lebih akurat namun saya menemukan sangat banyak permasalahan, mulai dari ketika menggunakan yolo26, program saya tetap crash ketika di colcon build
bahkan setelah diuji coba berkali kali dan mencari dari berbagai sumber untuk memperbaikinya.
Terakhir saya juga mendapati error di linux saya yang layarnya sering sekali flickering dan fps drop, dan jujur hingga sekarang belum bisa saya solve.
