#include <chrono>
#include <memory>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/int32.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

class NodeGame : public rclcpp::Node {
public:
    NodeGame() : Node("game_node"), total_skor_(0), timer_tahan_(0.0), penanda_waktu_lalu_(this->get_clock()->now()) {
        this->declare_parameter<double>("hold_duration", 2.0);
        this->declare_parameter<int>("min_area", 1500);

        berlangganan_gambar_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw", 10, std::bind(&NodeGame::proses_frame_game, this, std::placeholders::_1));
        
        publikasi_skor_ = this->create_publisher<std_msgs::msg::Int32>("/game/score", 10);
        publikasi_gambar_debug_ = this->create_publisher<sensor_msgs::msg::Image>("/game/debug_image", 10);
    }

private:
    void proses_frame_game(const sensor_msgs::msg::Image::SharedPtr pesan_masuk) {
        cv_bridge::CvImagePtr jembatan_cv;
        try {
            jembatan_cv = cv_bridge::toCvCopy(pesan_masuk, "bgr8");
        } catch (cv_bridge::Exception& error_log) {
            RCLCPP_ERROR(this->get_logger(), "Gagal mengubah format cv_bridge: %s", error_log.what());
            return;
        }

        cv::Mat gambar_game = jembatan_cv->image;

        rclcpp::Time penanda_waktu_sekarang = this->get_clock()->now();
        double delta_waktu = (penanda_waktu_sekarang - penanda_waktu_lalu_).seconds();
        penanda_waktu_lalu_ = penanda_waktu_sekarang;

        cv::Mat gambar_hsv, mask_emas, mask_le, hasil_masking;
        cv::cvtColor(gambar_game, gambar_hsv, cv::COLOR_BGR2HSV);
        
        cv::inRange(gambar_hsv, cv::Scalar(10, 80, 80), cv::Scalar(30, 255, 255), mask_emas);
        cv::inRange(gambar_hsv, cv::Scalar(80, 50, 50), cv::Scalar(130, 255, 255), mask_le);

        cv::bitwise_or(mask_emas, mask_le, hasil_masking);

        cv::morphologyEx(hasil_masking, hasil_masking, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
        cv::morphologyEx(hasil_masking, hasil_masking, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

        std::vector<std::vector<cv::Point>> kumpulan_kontur;
        cv::findContours(hasil_masking, kumpulan_kontur, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        double luas_maksimal = 0;
        int indeks_kontur_terbesar = -1;

        for (size_t idx = 0; idx < kumpulan_kontur.size(); idx++) {
            double luas_objek = cv::contourArea(kumpulan_kontur[idx]);
            if (luas_objek > luas_maksimal) {
                luas_maksimal = luas_objek;
                indeks_kontur_terbesar = idx;
            }
        }

        bool objek_terdeteksi = false;
        std::string nama_objek = "";

        if (indeks_kontur_terbesar != -1 && luas_maksimal > this->get_parameter("min_area").as_int()) {
            objek_terdeteksi = true;
            cv::Rect kotak_pembungkus = cv::boundingRect(kumpulan_kontur[indeks_kontur_terbesar]);
            
            cv::rectangle(gambar_game, kotak_pembungkus, cv::Scalar(0, 255, 0), 2);

            cv::Mat area_crop = hasil_masking(kotak_pembungkus);
            cv::Mat crop_emas = mask_emas(kotak_pembungkus);
            int piksel_emas = cv::countNonZero(crop_emas);
            int piksel_total = cv::countNonZero(area_crop);

            if (piksel_emas > (piksel_total * 0.5)) {
                nama_objek = "Tutup Emas";
            } else {
                nama_objek = "Tutup Le Minerale (Biru Muda)";
            }
            cv::putText(gambar_game, nama_objek, cv::Point(kotak_pembungkus.x, kotak_pembungkus.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        }

        double target_durasi_tahan = this->get_parameter("hold_duration").as_double();

        if (objek_terdeteksi) {
            timer_tahan_ += delta_waktu;
            
            int lebar_bar = (timer_tahan_ / target_durasi_tahan) * 200;
            if (lebar_bar > 200) lebar_bar = 200;
            
            cv::rectangle(gambar_game, cv::Point(20, 60), cv::Point(220, 75), cv::Scalar(50, 50, 50), cv::FILLED);
            cv::rectangle(gambar_game, cv::Point(20, 60), cv::Point(20 + lebar_bar, 75), cv::Scalar(0, 255, 255), cv::FILLED);
            cv::putText(gambar_game, "Mencatat Poin...", cv::Point(25, 72), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);

            if (timer_tahan_ >= target_durasi_tahan) {
                total_skor_++;
                std_msgs::msg::Int32 data_pesan_skor;
                data_pesan_skor.data = total_skor_;
                publikasi_skor_->publish(data_pesan_skor);
                timer_tahan_ = 0.0; 
            }
        } else {
            timer_tahan_ = 0.0;
        }

        cv::putText(gambar_game, "Skor Saat Ini: " + std::to_string(total_skor_), cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);

        auto pesan_gambar_debug = cv_bridge::CvImage(pesan_masuk->header, "bgr8", gambar_game).toImageMsg();
        publikasi_gambar_debug_->publish(*pesan_gambar_debug);
    }

    int total_skor_;
    double timer_tahan_;
    rclcpp::Time penanda_waktu_lalu_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr berlangganan_gambar_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publikasi_skor_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publikasi_gambar_debug_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NodeGame>());
    rclcpp::shutdown();
    return 0;
}
