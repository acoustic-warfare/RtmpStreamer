#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <mutex>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

class RtmpStreamer {
   public:
    /**
     * @brief Default constructor for the RtmpStreamer class.
     *
     * The `RtmpStreamer` constructor initializes the streamer with default
     * screen resolution of 1024x1024.
     */
    RtmpStreamer();

    /**
     * @brief Constructs an RtmpStreamer with specified width and height.
     *
     * @param width The width of the stream.
     * @param height The height of the stream.
     */
    RtmpStreamer(uint width, uint height);

    /**
     * @brief Deleted copy constructor to prevent copying of RtmpStreamer
     * instances.
     */
    RtmpStreamer(const RtmpStreamer &) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of
     * RtmpStreamer instances.
     *
     * This prevents assigning one RtmpStreamer instance to another.
     */
    RtmpStreamer &operator=(const RtmpStreamer &) = delete;

    /**
     * @brief Destructor for the RtmpStreamer class.
     *
     * Cleans up and releases resources held by the RtmpStreamer instance.
     */
    ~RtmpStreamer();

    /**
     * @brief Sends a video frame to the GStreamer pipeline for streaming.
     *
     * @param frame The video frame to be sent, represented as an OpenCV Mat
     * object.
     * @return True if the frame was successfully sent; false otherwise.
     */
    bool send_frame(cv::Mat &frame);

    /**
     * @brief Sends a video frame to the GStreamer pipeline for streaming.
     *
     * @param frame Pointer to the raw video frame data.
     * @param size The size of the video frame data in bytes.
     * @return True if the frame was successfully sent; false otherwise.
     */
    bool send_frame(unsigned char *frame, size_t size);

    /**
     * @brief Starts the hole streaming pipeline.
     */
    void start_stream();

    /**
     * @brief Stops the hole streaming pipeline.
     */
    void stop_stream();

    /**
     * @brief Starts the RTMP streaming.
     */
    void start_rtmp_stream();

    /**
     * @brief Stops the RTMP streaming.
     */
    void stop_rtmp_stream();

    /**
     * @brief Starts the local video stream.
     */
    void start_local_stream();

    /**
     * @brief Stops the local video stream.
     */
    void stop_local_stream();

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

    /**
     * @brief Connects a sink bin to a source bin in a GStreamer pipeline.
     *
     * This function adds a sink bin to the GStreamer pipeline and links it to a
     * source bin using a tee element. Does not change pipeline state but sets
     * sink element to GST_PLAYING
     *
     * @param source_bin The source bin element to connect from.
     * @param sink_bin The sink bin element to connect to.
     * @param request_pad A pointer to store the requested pad from the tee
     * element.
     * @param tee_element_name The name of the tee element within the source
     * bin.
     * @param tee_ghost_pad_name The name of the ghost pad to create in the
     * source bin.
     * @return true if the connection was successful, false otherwise.
     *
     * NOTE: Source element must contain a tee element.
     */
    bool connect_sink_bin_to_source_bin(GstElement *source_bin,
                                        GstElement *sink_bin,
                                        GstPad **request_pad,
                                        const char *tee_element_name,
                                        const char *tee_ghost_pad_name);

    /**
     * @brief Disconnects a sink bin from a source bin in the GStreamer
     * pipeline.
     *
     * @param source_bin The source bin from which to disconnect.
     * @param sink_bin The sink bin to disconnect.
     * @param tee_pad The pad from the tee element to be disconnected.
     * @param tee_ghost_pad_src_name The name of the ghost pad in the source
     * bin.
     * @return True if the disconnection is successful, otherwise false.
     */
    bool disconnect_sink_bin_from_source_bin(
        GstElement *source_bin, GstElement *sink_bin, GstPad *tee_pad,
        const char *tee_ghost_pad_src_name);

    /**
     * @brief Sends a frame to the appsrc element.
     *
     * @param data Pointer to the frame data. Must be in RGB format.
     * @param size Size of the frame data in bytes.
     * @return True if the frame is successfully sent, otherwise false.
     */
    bool send_frame_to_appsrc(void *data, size_t size);

    /**
     * @brief Checks for errors in the streaming process.
     *
     * @return True if there is an error, otherwise false.
     */
    [[nodiscard]] bool check_error() const;
    gboolean check_links();
    void initialize_streamer();

    static std::mutex want_data_muxex;
    static std::mutex handling_pipeline;

    uint screen_width, screen_height;
    bool want_data;
    int connected_bins_to_source;
    GstElement *pipeline, *source_bin, *rtmp_bin, *local_video_bin;
    gchar *source_bin_name, *rtmp_bin_name, *local_video_bin_name;
    GstElement *appsrc;
    GstPad *src_rtmp_tee_pad, *src_local_tee_pad;

    gint appsrc_need_data_id;
    gint appsrc_enough_data_id;
    GstBus *bus;
};
