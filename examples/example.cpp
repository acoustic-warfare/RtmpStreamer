#include <future>
#include <rtmp.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
typedef cv::Point3_<uint8_t> Pixel;

int main(int argc, char *argv[]) {
    // Initialize the Streamer with a width of 1920 and height of 1080.
    RtmpStreamer streamer(SCREEN_WIDTH, SCREEN_HEIGHT);
    streamer.start_stream();

    // Generate a frame and colormap it
    cv::Mat frame(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC1);
    cv::applyColorMap(frame, frame, cv::COLORMAP_JET);

    // This control unit takes input from the terminal and controls the state of the streamer
    auto control_unit =
        std::async(std::launch::async,
                   &RtmpStreamer::async_streamer_control_unit, &streamer);

    // do some color manipulation on the frame
    frame.forEach<Pixel>([](Pixel &pix, const int *position) {
        pix.x = 255;
        pix.y = 0;
        pix.z = 0;
    });

    while (true) {
        // Pass the frame on to the streamer pipeline to be shown locally and/or sent up to waraps.
        // Streamer does not take ownership of the frame and does not change anything in the frame.
        streamer.send_frame(frame);

        // Only returns when user has typed "quit" in the terminal
        if (control_unit.wait_for(std::chrono::milliseconds(10)) ==
            std::future_status::ready) {
            break;
        }
    }

    return 0;
}
