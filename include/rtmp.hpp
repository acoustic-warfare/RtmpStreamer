#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <cstdint>
#include <mutex>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

class RtmpStreamer {
   public:
    GstBus *bus;

    /**
     * @brief Default constructor for the RtmpStreamer class.
     *
     * The `RtmpStreamer` constructor initializes the streamer with default
     * screen dimensions and sets the `want_data` flag to `false`. It then calls
     * `initialize_streamer` to set up the GStreamer pipeline and elements.
     *
     * - Sets the screen width to 1024 pixels.
     * - Sets the screen height to 1024 pixels.
     * - Initializes the `want_data` flag to `false`.
     * - Calls the `initialize_streamer` function to configure the streaming
     * pipeline.
     */
    RtmpStreamer();
    RtmpStreamer(uint width, uint height);
    RtmpStreamer(const RtmpStreamer &) = delete;
    RtmpStreamer &operator=(const RtmpStreamer &) = delete;
    ~RtmpStreamer();

    bool send_frame(cv::Mat frame);
    bool send_frame(uint8_t *frame, size_t size);

    void start_stream();
    void stop_stream();

    void start_rtmp_stream();
    void stop_rtmp_stream();
    void start_local_stream();
    void stop_local_stream();

    [[nodiscard]] bool check_error() const;

    /**
     * @brief Provides a command-line interface for controlling the RTMP and
     * local streams.
     *
     * The `async_streamer_control_unit` function continuously reads commands
     * from the standard input and executes the corresponding streamer control
     * functions based on the input.
     *
     * Supported commands:
     * - `stop_stream`        : Stops the whole stream.
     * - `start_stream`       : Stops the whole stream.
     * - `stop_rtmp_stream`   : Stops the RTMP stream.
     * - `stop_local_stream`  : Stops the local stream.
     * - `start_rtmp_stream`  : Starts the RTMP stream.
     * - `start_local_stream` : Starts the local stream.
     * - `quit`               : Exits the command loop.
     *
     * If an invalid command is entered, an error message is printed to the
     * standard error. The command loop will continue to process commands until
     * the "quit" command is received.
     */
    void async_streamer_control_unit();
    void debug_info();

   private:
    /**
     * @brief Callback function that sets the flag indicating that data is
     * needed.
     *
     * The `cb_need_data` function is a static callback used by the GStreamer
     * `appsrc` element. It is called when the pipeline needs more data. The
     * function sets the `want_data` flag to `true` to indicate that the
     * application should provide more data.
     *
     * @param appsrc The GStreamer appsrc element requesting data.
     * @param size The size of the data needed.
     * @param user_data A pointer to user data; in this case, a boolean flag
     * indicating the need for data.
     */
    static void cb_need_data(GstAppSrc *appsrc, guint size, gpointer user_data);

    /**
     * @brief Callback function that resets the flag indicating that no more
     * data is needed.
     *
     * The `cb_enough_data` function is a static callback used by the GStreamer
     * `appsrc` element. It is called when the pipeline has enough data. The
     * function sets the `want_data` flag to `false` to indicate that the
     * application should stop providing data.
     *
     * @param appsrc The GStreamer appsrc element indicating no more data is
     * needed.
     * @param user_data A pointer to user data; in this case, a boolean flag
     * indicating the need for data.
     */
    static void cb_enough_data(GstAppSrc *appsrc, gpointer user_data);

    bool connect_sink_bin_to_source_bin(GstElement *source_bin,
                                        GstElement *sink_bin,
                                        GstPad **request_pad,
                                        const char *tee_element_name,
                                        const char *tee_ghost_pad_name);
    bool disconnect_sink_bin_from_source_bin(
        GstElement *source_bin, GstElement *sink_bin, GstPad *tee_pad,
        const char *tee_ghost_pad_src_name);

    bool send_frame_to_appsrc(void *data, size_t size);

    gboolean check_links();
    void initialize_streamer();

    GstElement *pipeline, *source_bin, *rtmp_bin, *local_video_bin;
    gchar *source_bin_name, *rtmp_bin_name, *local_video_bin_name;
    GstElement *appsrc;
    GstPad *src_rtmp_tee_pad, *src_local_tee_pad;

    bool want_data;
    uint screen_width, screen_height;
    static std::mutex want_data_muxex;
    static std::mutex handling_pipeline;
};
