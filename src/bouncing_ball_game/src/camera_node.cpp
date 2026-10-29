#include <chrono>
#include <memory>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

using namespace std::chrono_literals;

class NodeKamera : public rclcpp::Node {
public:
    NodeKamera() : Node("camera_node") {
        this->declare_parameter<std::string>("source", "0");
        this->declare_parameter<double>("fps", 30.0);
        this->declare_parameter<int>("width", 640);
        this->declare_parameter<int>("height", 480);

        double fps_kamera = this->get_parameter("fps").as_double();
        int lebar_frame = this->get_parameter("width").as_int();
        int tinggi_frame = this->get_parameter("height").as_int();

        // Otomatis mencari indeks kamera yang aktif di laptopmu
        std::vector<int> daftar_indeks = {0, 1, 2, 4};
        bool berhasil_buka = false;

        for (int indeks : daftar_indeks) {
            tangkap_video_.open(indeks);
            if (tangkap_video_.isOpened()) {
                RCLCPP_INFO(this->get_logger(), "Berhasil membuka webcam pada indeks: %d", indeks);
                berhasil_buka = true;
                break;
            }
        }

        if (!berhasil_buka) {
            RCLCPP_ERROR(this->get_logger(), "Waduh, semua indeks webcam gagal dibuka!");
            return;
        }

        tangkap_video_.set(cv::CAP_PROP_FRAME_WIDTH, lebar_frame);
        tangkap_video_.set(cv::CAP_PROP_FRAME_HEIGHT, tinggi_frame);

        publikasi_gambar_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/image_raw", 10);

        int jeda_waktu_ms = static_cast<int>(1000.0 / fps_kamera);
        timer_kamera_ = this->create_wall_timer(
            std::chrono::milliseconds(jeda_waktu_ms), 
            std::bind(&NodeKamera::siklus_baca_kamera, this)
        );
    }

private:
    void siklus_baca_kamera() {
        cv::Mat frame_mentah;
        tangkap_video_ >> frame_mentah;

        if (!frame_mentah.empty()) {
            auto jembatan_cv = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame_mentah);
            auto pesan_gambar = jembatan_cv.toImageMsg();
            publikasi_gambar_->publish(*pesan_gambar);
        }
    }

    cv::VideoCapture tangkap_video_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publikasi_gambar_;
    rclcpp::TimerBase::SharedPtr timer_kamera_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NodeKamera>());
    rclcpp::shutdown();
    return 0;
}
