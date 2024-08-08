#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <mutex>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <string>

class RtmpStreamer {
   public:

    /**
     * @brief Default constructor for the RtmpStreamer class.
     *
     * The `RtmpStreamer` constructor initializes the streamer with a default
     * video input resolution of 1024x1024. Default RTMP server is
     * 'rtmp://ome.waraps.org/app/name-your-stream'. 
     */
    RtmpStreamer();

    /**
     * @brief Constructs an RtmpStreamer with specified width and height.
     *
     * @param width The pixel width of each input frame.
     * @param height The pixel height of each input frame.
     * @param rtmp_streaming_addr The address of the RTMP server to stream to.
     *
     * NOTE: The last part of the streaming address becomes the name of stream
     *
     */
    RtmpStreamer(uint width, uint height, const char *rtmp_streaming_addr);

    /**
     * @brief Deleted copy constructor to prevent copying of RtmpStreamer
     * instances.
     */
    RtmpStreamer(const RtmpStreamer &) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of
     * RtmpStreamer instances.
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
     * The cv::Mat frame must be in the format of either RGBA or ARGB.
     *
     * @param frame The video frame to be sent, represented as an OpenCV Mat
     * object.
     * @return True if the frame was successfully sent; false otherwise.
     */
    bool send_frame(cv::Mat &frame);

    /**
     * @brief Sends a video frame to the GStreamer pipeline for streaming.
     *
     * The Frame must be in the format of RGB.
     *
     * @param frame Pointer to the raw video frame data.
     * @param size The size of the video frame data in bytes.
     * @return True if the frame was successfully sent; false otherwise.
     */
    bool send_frame(unsigned char *frame, size_t size);

    /**
     * @brief Starts the whole streaming pipeline.
     *
     * Will start both the local stream and the stream to the RTMP server.
     */
    void start_stream();

    /**
     * @brief Stops the whole streaming pipeline.
     *
     * Will stop both the local stream and the stream to the RTMP server.
     */
    void stop_stream();

    /**
     * @brief Starts the stream to the RTMP server.
     */
    void start_rtmp_stream();

    /**
     * @brief Stops the stream to the RTMP server.
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

    /**
     * @brief Provides information about all elements connected to the pipeline.
     *
     * A debuging tool to display information about the pipeline such as,
     * available elements, available pads, ghost pads and their linking state.
     * It also gives information abot the current state of the element.
     */
    void debug_info();

   private:
    /**
     * @brief Callback function that sets the flag indicating that data is
     * needed.
     *
     * The `cb_need_data` function is a static callback used by the GStreamer
     * `appsrc` element. It is called by the appsrc element when it needs more
     * data.
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
     * `appsrc` element. It is called by appsrc when it has enough data.
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
     * This function adds a sink bin to the pipeline and links it to a
     * source bin using a tee element. Does not change pipeline state but sets
     * sink element to GST_PLAYING
     *
     * NOTE: Source element must contain a tee element.
     *
     * @param source_bin The source bin element to connect from.
     * @param sink_bin The sink bin element to connect to. 
     * NOTE: Transfers ownership of sink_bin to the pipeline
     *
     * @param request_pad A place to store the requested pad from the tee
     * element.
     * @param tee_element_name The name of the tee element within the source
     * bin.
     * @param tee_ghost_pad_name The name of the ghost pad to create in the
     * source bin.
     * @return true if the connection was successful, false otherwise.
     */
    bool connect_sink_bin_to_source_bin(GstElement *source_bin,
                                        GstElement **sink_bin,
                                        GstPad **request_pad,
                                        const char *tee_element_name,
                                        const char *tee_ghost_pad_name);

    /**
     * @brief Disconnects a sink bin from a source bin in the GStreamer
     * pipeline.
     *
     * @param source_bin The source bin from which to disconnect.
     * @param sink_bin Location to store the removed sink-bin
     * @param request_pad The pad from the tee element to be disconnected.
     * @param sink_bin_name The name of the sink bin to remove
     * @param tee_ghost_pad_name The name of the ghost pad in the source
     * bin.
     * @return True if the disconnection is successful, otherwise false.
     *
     * @side-effect Transfers ownership of removed sink-bin to 
     */
    bool disconnect_sink_bin_from_source_bin(GstElement *source_bin,
                                             GstElement **sink_bin,
                                             GstPad *request_pad,
                                             const char *sink_bin_name,
                                             const char *tee_ghost_pad_name);

    /**
     * @brief Sends a frame to the appsrc element.
     *
     * @param data Pointer to the frame data. Must be in RGB format.
     * @param size Size of the frame data in bytes.
     * @return True if the frame is successfully sent, otherwise false.
     */
    bool send_frame_to_appsrc(void *data, size_t size);

    /**
     * @brief Connects the signal handlers for appsrc's need-data and
     * enough-data signals.
     *
     * @return True if the signal handlers are successfully connected, false
     * otherwise.
     */
    bool connect_appsrc_signal_handler();

    /**
     * @brief Disconnects the signal handlers for appsrc's need-data and
     * enough-data signals.
     *
     * @return True if the signal handlers are successfully disconnected, false
     * otherwise.
     */
    bool disconnect_appsrc_signal_handler();

    /**
     * @brief Checks for errors in the streaming process.
     *
     * NOTE: Should be run async with the program.
     *
     * @return True if there is an error, otherwise false.
     */
    [[nodiscard, maybe_unused]] bool check_error() const;
    gboolean check_links();
    void initialize_streamer();

    /**
     * @brief Mutex for synchronizing access to the want_data flag.
     */
    static std::mutex want_data_muxex;

    /**
     * @brief Mutex for synchronizing access to the pipeline handling process.
     */
    static std::mutex handling_pipeline;

    /**
     * @brief The width of the screen or video frame.
     */
    uint screen_width;

    /**
     * @brief The height of the screen or video frame.
     */
    uint screen_height;

    /**
     * @brief Flag indicating whether data is needed by appsrc.
     */
    bool want_data;

    /**
     * @brief The number of bins currently connected to the source bin.
     */
    int connected_bins_to_source;

    /**
     * @brief The GStreamer pipeline element.
     */
    GstElement *pipeline;

    /**
     * @brief The source bin element in the GStreamer pipeline.
     */
    GstElement *source_bin;

    /**
     * @brief The RTMP bin element in the GStreamer pipeline.
     */
    GstElement *rtmp_bin;

    /**
     * @brief The local video bin element in the GStreamer pipeline.
     */
    GstElement *local_video_bin;

    /**
     * @brief The name of the source bin element.
     */
    gchar *source_bin_name;

    /**
     * @brief The name of the RTMP bin element.
     */
    gchar *rtmp_bin_name;

    /**
     * @brief The name of the local video bin element.
     */
    gchar *local_video_bin_name;

    /**
     * @brief The appsrc element for pushing frames into the GStreamer pipeline.
     */
    GstElement *appsrc;

    /**
     * @brief The pad of the RTMP tee element.
     */
    GstPad *src_rtmp_tee_pad;

    /**
     * @brief The pad of the local video tee element.
     */
    GstPad *src_local_tee_pad;

    /**
     * @brief ID for the need-data signal handler for appsrc.
     */
    gint appsrc_need_data_id;

    /**
     * @brief ID for the enough-data signal handler appsrc.
     */
    gint appsrc_enough_data_id;

    /**
     * @brief The GStreamer bus element for handling messages from the pipeline.
     */
    GstBus *bus;

    /**
     * @brief The address for RTMP streaming.
     */
    std::string rtmp_streaming_addr;
};
