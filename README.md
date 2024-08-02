# Overview
Dynamic RTMP streamer. Library making it possible to dynamically at runtime start and stop streaming RTMP video to  a RTMP-server as well as displaying the video locally locally. 

# Dependencies

To build and run the DynRT project, you need to have the following dependencies installed:

## Build System and Language Requirements
- **Meson Build System**: Version 0.63.0 or higher
- **Ninja Build System**: Version 1.10.0 or higher
- **g++ compiler***: 
- **C++**: C++17 standard
- **Python**: Version 3.8 or higher

## Libraries and Packages
- **GStreamer 1.0**: Version 1.0 or higher
- **OpenCV 4**: Version 4.0 or higher
- **Threads**: Required for multithreading support
- **GStreamer App 1.0**: Version 1.0 or higher
- **fmt**: Version 7.1.3 or higher

## Python Packages
- **Cython**: Version 0.29.21 or higher
- **NumPy**: Version 1.19.2 or higher

## How to Install Dependencies

### Using Package Managers

#### Ubuntu
**(NOTE: depending on ubuntu version, meson and ninja might be to old and need to be installed )**
```bash
sudo apt-get update
sudo apt-get install -y g++ gstreamer1.0 libgstreamer1.0-dev libopencv-dev libfmt-dev python3-dev python3-pip
python3 -m pip install cython numpy meson ninja
```

## From Source
A lot of the libraries can also be installed from source. Here are a few links to their installation instructions:
- [meson](https://github.com/mesonbuild/meson)
- [ninja](https://github.com/ninja-build/ninja)
- [gstreamer](https://github.com/GStreamer/gstreamer)
- [opencv](https://github.com/opencv/opencv/tree/4.10.0)
- [fmt](https://github.com/fmtlib/fmt)

# Install DynRT
The library can be installed with the following series of commands:
```
git clone https://github.com/acoustic-warfare/DynRT-streamer && \
cd DynRT-streamer
meson setup build --native-file native-file.ini
ninja -C build
sudo ninja -C build install
```


# Usecase
*C++* usecase with comments:
```c++
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
```






