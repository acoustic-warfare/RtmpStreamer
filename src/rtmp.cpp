#include "rtmp.hpp"

#include <mutex>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <string>


#define RGB_BYTES 3
std::mutex RtmpStreamer::want_data_muxex = std::mutex();
std::mutex RtmpStreamer::handling_pipeline = std::mutex();

/**
 * @brief Provides a command-line interface for controlling the RTMP and local
 * streams.
 *
 * The `async_streamer_control_unit` function continuously reads commands from
 * the standard input and executes the corresponding streamer control functions
 * based on the input.
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
 * If an invalid command is entered, an error message is printed to the standard
 * error. The command loop will continue to process commands until the "quit"
 * command is received.
 */
void RtmpStreamer::async_streamer_control_unit() {
    std::string command;
    std::getline(std::cin, command);

    do {
        if (std::strcmp(command.c_str(), "stop_rtmp_stream") == 0) {
            stop_rtmp_stream();
        } else if (std::strcmp(command.c_str(), "stop_local_stream") == 0) {
            stop_local_stream();
        } else if (std::strcmp(command.c_str(), "start_rtmp_stream") == 0) {
            start_rtmp_stream();
        } else if (std::strcmp(command.c_str(), "start_local_stream") == 0) {
            start_local_stream();
        } else if (std::strcmp(command.c_str(), "quit") == 0) {
            break;
        } else {
            gst_printerr("\nInvalid command.\n");
        }
    } while (std::getline(std::cin, command));
}

/**
 * @brief Callback function that sets the flag indicating that data is needed.
 *
 * The `cb_need_data` function is a static callback used by the GStreamer
 * `appsrc` element. It is called when the pipeline needs more data. The
 * function sets the `want_data` flag to `true` to indicate that the application
 * should provide more data.
 *
 * @param appsrc The GStreamer appsrc element requesting data.
 * @param size The size of the data needed.
 * @param user_data A pointer to user data; in this case, a boolean flag
 * indicating the need for data.
 */
void RtmpStreamer::cb_need_data(GstAppSrc *appsrc, guint size,
                                gpointer user_data) {
    std::lock_guard<std::mutex> guard(want_data_muxex);
    bool *want_data = (bool *)user_data;
    *want_data = true;
}

/**
 * @brief Callback function that resets the flag indicating that no more data is
 * needed.
 *
 * The `cb_enough_data` function is a static callback used by the GStreamer
 * `appsrc` element. It is called when the pipeline has enough data. The
 * function sets the `want_data` flag to `false` to indicate that the
 * application should stop providing data.
 *
 * @param appsrc The GStreamer appsrc element indicating no more data is needed.
 * @param user_data A pointer to user data; in this case, a boolean flag
 * indicating the need for data.
 */
void RtmpStreamer::cb_enough_data(GstAppSrc *appsrc, gpointer user_data) {
    std::lock_guard<std::mutex> guard(want_data_muxex);
    bool *want_data = (bool *)user_data;
    *want_data = false;
}

/**
 * @brief Default constructor for the RtmpStreamer class.
 *
 * The `RtmpStreamer` constructor initializes the streamer with default screen
 * dimensions and sets the `want_data` flag to `false`. It then calls
 * `initialize_streamer` to set up the GStreamer pipeline and elements.
 *
 * - Sets the screen width to 1024 pixels.
 * - Sets the screen height to 1024 pixels.
 * - Initializes the `want_data` flag to `false`.
 * - Calls the `initialize_streamer` function to configure the streaming
 * pipeline.
 */
RtmpStreamer::RtmpStreamer()
    : screen_width(1024), screen_height(1024), want_data(false) {
    initialize_streamer();
}

/**
 * @brief Parameterized constructor for the RtmpStreamer class.
 *
 * The `RtmpStreamer` constructor initializes the streamer with specified screen
 * dimensions and sets the `want_data` flag to `false`. It then calls
 * `initialize_streamer` to set up the GStreamer pipeline and elements.
 *
 * @param width The desired screen width in pixels.
 * @param height The desired screen height in pixels.
 *
 * - Sets the screen width to the provided `width` value.
 * - Sets the screen height to the provided `height` value.
 * - Initializes the `want_data` flag to `false`.
 * - Calls the `initialize_streamer` function to configure the streaming
 * pipeline.
 */
RtmpStreamer::RtmpStreamer(uint width, uint height)
    : screen_width(width), screen_height(height), want_data(false) {
    initialize_streamer();
}

/**
 * @brief Destructor for the RtmpStreamer class.
 *
 * The `~RtmpStreamer` destructor cleans up the GStreamer pipeline and releases
 * resources. It sets the pipeline state to `GST_STATE_NULL` to stop any ongoing
 * streaming and unrefs the pipeline object to free allocated memory.
 */
RtmpStreamer::~RtmpStreamer() {
    g_print("set to null.\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

/**
 * @brief Starts the streaming pipeline and prepares the appsrc element.
 *
 * The `start_stream` function initializes the `appsrc` element, sets up signal
 * connections for data handling, and transitions the pipeline to the
 * `GST_STATE_PLAYING` state to start streaming.
 *
 * Edge Cases:
 * - If the `appsrc` element cannot be retrieved from `source_bin`, an error
 * message is printed and the program exits.
 */
void RtmpStreamer::start_stream() {
    appsrc = gst_bin_get_by_name(GST_BIN(source_bin), "appsrc");
    if (!appsrc) {
        gst_printerr("error extracting appsrc\n");
        exit(1);
    }
    g_signal_connect(appsrc, "need-data", G_CALLBACK(cb_need_data), &want_data);
    g_signal_connect(appsrc, "enough-data", G_CALLBACK(cb_enough_data),
                     &want_data);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bus = gst_element_get_bus(pipeline);
}

/**
 * @brief Stops the streaming pipeline.
 *
 * The `stop_stream` function sets the state of the pipeline to
 * `GST_STATE_NULL`, which stops the streaming process and releases the
 * resources used by the pipeline.
 */
void RtmpStreamer::stop_stream() {
    gst_element_set_state(pipeline, GST_STATE_NULL);
}

bool RtmpStreamer::send_frame_to_appsrc(void *data, size_t size) {
    static guint64 count = 0;
    GstBuffer *buffer;
    GstFlowReturn ret;

    // Create a new buffer
    buffer = gst_buffer_new_allocate(nullptr, size, nullptr);

    GstClock *clock = gst_element_get_clock(appsrc);
    if (clock) {
        GstClockTime base_time = gst_element_get_base_time(appsrc);
        GstClockTime current_time = gst_clock_get_time(clock);
        GstClockTime running_time = current_time - base_time;
        GstClockTime timestamp = running_time;

        // Set the PTS (presentation timestamp) and DTS (decoding timestamp) of
        // the buffer
        GST_BUFFER_PTS(buffer) = timestamp;
        GST_BUFFER_DTS(buffer) = timestamp;
        GST_BUFFER_DURATION(buffer) =
            (GstClockTime)gst_util_uint64_scale_int(GST_SECOND, 1, 30);
        gst_object_unref(clock);
    } else {
        gst_printerr("unable to open clock for appsrc!\n");
        exit(1);
    }

    if (!GST_BUFFER_DURATION_IS_VALID(buffer)) {
        gst_printerr("Invalid buffer duration.!\n");
        exit(1);
    }
    count += GST_SECOND / 30;

    // Copy the cv::Mat data into the GStreamer buffer
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, data, size);
    gst_buffer_unmap(buffer, &map);

    // Push the buffer to appsrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    // Free the buffer
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        // We got some error, stop sending data
        g_print("error when sending :(.\n");
        return FALSE;
    }

    return TRUE;
}

bool RtmpStreamer::send_frame(uint8_t *frame, size_t size) {
    if (size <= 0) {
        g_printerr("Captured frame is empty.\n");
        return FALSE;
    }

    std::lock_guard<std::mutex> guard(handling_pipeline);

    want_data_muxex.lock();
    if (!want_data) {
        // gst_printerr("appsrc does not require data right now.\n");
        want_data_muxex.unlock();
        return FALSE;
    }
    want_data_muxex.unlock();

    return send_frame_to_appsrc((void *)frame, size);
}

bool RtmpStreamer::send_frame(cv::Mat frame) {
    if (frame.empty()) {
        g_printerr("Captured frame is empty.\n");
        return FALSE;
    }

    std::lock_guard<std::mutex> guard(handling_pipeline);

    want_data_muxex.lock();
    if (!want_data) {
        // gst_printerr("appsrc does not require data right now.\n");
        want_data_muxex.unlock();
        return FALSE;
    }
    want_data_muxex.unlock();

    // Ensure the frame is in RGB format
    if (frame.channels() == 4) {
        cv::cvtColor(frame, frame, cv::COLOR_BGRA2RGB);
    } else if (frame.channels() == 3) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    } else {
        g_printerr("Captured frame is not in a supported format.\n");
        return FALSE;
    }

    if (!send_frame_to_appsrc(&frame.data, frame.total() * frame.elemSize())) {
        return false;
    }
}

/* @brief Sets up the GStreamer pipeline and its elements for streaming.
 *
 * The `initialize_streamer` function configures the GStreamer pipeline with
 * necessary bins and elements, and links them to enable streaming. It creates
 * and sets up:
 * - A source bin with `appsrc`, video conversion, scaling, and tee elements.
 * - An RTMP bin with encoding, muxing, and RTMP sink elements.
 * - A local video bin with a queue and video sink element.
 *
 * Edge Cases:
 * - If any element creation or bin setup fails, an error message is printed and
 * the program exits.
 * - If the `tee` element cannot be retrieved, an error message is printed and
 * the program exits.
 * - If linking pads between elements fails, an error message is printed and the
 * program exits.
 */
void RtmpStreamer::initialize_streamer() {
    gst_init(nullptr, nullptr);

    pipeline = gst_pipeline_new("video pipeline");

    if (!pipeline) {
        gst_printerr("unable to create pipeline\n");
        exit(1);
    }

    source_bin = gst_parse_bin_from_description(
        "appsrc name=appsrc is-live=true block=false "
        "format=GST_FORMAT_TIME "
        "caps=video/x-raw,format=RGB,framerate=60/1,width=1024,height=1024 "
        "! videoconvert name=videoconvert ! videoscale name=videoscale ! "
        "videorate name=videorate ! "
        "video/x-raw,framerate=30/1,width=512,height=512 ! tee name=tee",
        false, nullptr);

    source_bin_name = gst_element_get_name(source_bin);

    rtmp_bin = gst_parse_bin_from_description(
        "x264enc name=x264_encoder tune=zerolatency speed-preset=superfast "
        "bitrate=2500 ! "
        "queue name=rtmp_queue ! flvmux name=flvmux streamable=true ! "
        "rtmpsink "
        "name=rtmp_sink"
        " location=rtmp://ome.waraps.org/app/beamforming",
        true, nullptr);
    rtmp_bin_name = gst_element_get_name(rtmp_bin);

    local_video_bin = gst_parse_bin_from_description(
        "queue name=local_video_queue ! glimagesink "
        "name=local_video_sink",
        true, nullptr);
    local_video_bin_name = gst_element_get_name(local_video_bin);

    if (!source_bin || !rtmp_bin || !local_video_bin) {
        gst_printerrln("Error setting up bins.");
        exit(1);
    }

    gst_bin_add_many(GST_BIN(pipeline), source_bin, rtmp_bin, local_video_bin,
                     NULL);

    GstElement *tee = gst_bin_get_by_name(GST_BIN(source_bin), "tee");
    if (!tee) {
        gst_printerrln("Unable to retreive tee element.");
        exit(1);
    }

    src_rtmp_tee_pad = gst_element_request_pad_simple(tee, "src_%u");
    src_local_tee_pad = gst_element_request_pad_simple(tee, "src_%u");

    gst_element_add_pad(source_bin,
                        gst_ghost_pad_new("tee_rtmp_src", src_rtmp_tee_pad));
    gst_element_add_pad(
        source_bin, gst_ghost_pad_new("local_video_src", src_local_tee_pad));

    GstPad *source_bin_rtmp =
        gst_element_get_static_pad(source_bin, "tee_rtmp_src");
    GstPad *rtmp_sink_pad = gst_element_get_static_pad(rtmp_bin, "sink");
    GstPadLinkReturn rtmp_link_result =
        gst_pad_link(source_bin_rtmp, rtmp_sink_pad);
    if (rtmp_link_result != GST_PAD_LINK_OK) {
        gst_printerrln(
            "Unable to link request-pads to video and rtmp bins. rtmp "
            "result: %d.",
            rtmp_link_result);
        exit(1);
    }

    GstPad *source_bin_video =
        gst_element_get_static_pad(source_bin, "local_video_src");
    GstPad *local_video_sink_pad =
        gst_element_get_static_pad(local_video_bin, "sink");
    GstPadLinkReturn video_link_result =
        gst_pad_link(source_bin_video, local_video_sink_pad);

    if (video_link_result != GST_PAD_LINK_OK) {
        gst_printerrln(
            "Unable to link request-pads to video and rtmp bins.video "
            "result: %d.",
            video_link_result);
        exit(1);
    }

    gst_object_unref(source_bin_rtmp);
    gst_object_unref(source_bin_video);
    gst_object_unref(rtmp_sink_pad);
    gst_object_unref(local_video_sink_pad);
}

/**
 * @brief Sets the state of a GStreamer element to match its parent's state.
 *
 * The `set_element_state_to_parent_state` function retrieves the state of the
 * parent element and applies that state to the specified element. This ensures
 * that the element's state aligns with the parent element's state, facilitating
 * consistent pipeline behavior.
 *
 * @param element The GStreamer element whose state is to be set.
 *
 * Edge Cases:
 * - If the `element` is `NULL`, an error message is printed.
 * - If the `element` has no parent, an error message is printed.
 * - If retrieving the parent's state fails, an error message is printed.
 * - If setting the element's state fails, an error message is printed.
 */
static void set_element_state_to_parent_state(GstElement *element) {
    GstElement *parent;
    GstState parent_state, parent_pending;
    GstStateChangeReturn ret;

    if (!element) {
        g_printerr("Element is NULL\n");
        return;
    }

    // Get the parent element
    parent = GST_ELEMENT(gst_element_get_parent(element));
    if (!parent) {
        g_printerr("Element has no parent\n");
        return;
    }

    // Get the current state and pending state of the parent
    ret = gst_element_get_state(parent, &parent_state, &parent_pending,
                                GST_CLOCK_TIME_NONE);
    if (ret != GST_STATE_CHANGE_SUCCESS) {
        g_printerr("Failed to get parent state\n");
        gst_object_unref(parent);
        return;
    }

    // Set the state of the element to the parent state
    ret = gst_element_set_state(element, parent_state);
    if (ret != GST_STATE_CHANGE_SUCCESS) {
        g_printerr("Failed to set element state\n");
    } else {
        g_print("Element state set to match parent state: %d\n", parent_state);
    }

    // Clean up
    gst_object_unref(parent);
}

/**
 * @brief Starts the RTMP streaming by connecting the RTMP bin to the source
 * bin.
 *
 * The `start_rtmp_stream` function checks if the RTMP bin is already connected
 * to the pipeline. If not, it connects the RTMP bin to the source bin using the
 * `connect_sink_bin_to_source_bin` method.
 *
 * Edge Cases:
 * - If the RTMP bin is already connected, a message is printed and the function
 * returns without making changes.
 * - If connecting the RTMP bin to the source bin fails, the program exits.
 */
void RtmpStreamer::start_rtmp_stream() {
    GstElement *bin = gst_bin_get_by_name(GST_BIN(pipeline), rtmp_bin_name);
    if (bin) {
        gst_print("rtmp bin already connected\n");
        g_object_unref(bin);
        return;
    }
    gst_print("hello");
    if (!connect_sink_bin_to_source_bin(source_bin, rtmp_bin, &src_rtmp_tee_pad,
                                        "tee", "tee_rtmp_src")) {
        exit(1);
    }
}

/**
 * @brief Stops the RTMP streaming by disconnecting the RTMP bin from the source
 * bin.
 *
 * The `stop_rtmp_stream` function checks if the RTMP bin is connected to the
 * pipeline. If it is, the function disconnects the RTMP bin from the source bin
 * and updates internal state.
 *
 * Edge Cases:
 * - If the RTMP bin is already disconnected, a message is printed and the
 * function returns without making changes.
 * - If disconnecting the RTMP bin from the source bin fails, the program exits.
 * - The `src_rtmp_tee_pad` is set to `nullptr` after disconnection to avoid
 * dangling pointers.
 */
void RtmpStreamer::stop_rtmp_stream() {
    GstElement *bin = gst_bin_get_by_name(GST_BIN(pipeline), rtmp_bin_name);
    if (!bin) {
        gst_print("rtmp bin already disconnected\n");
        return;
    }
    g_object_unref(bin);
    if (!disconnect_sink_bin_from_source_bin(
            source_bin, rtmp_bin, src_rtmp_tee_pad, "tee_rtmp_src")) {
        exit(1);
    }
    src_rtmp_tee_pad = nullptr;
}

/**
 * @brief Starts the local video streaming by connecting the local video bin to
 * the source bin.
 *
 * The `start_local_stream` function checks if the local video bin is already
 * connected to the pipeline. If not, it connects the local video bin to the
 * source bin using the `connect_sink_bin_to_source_bin` method.
 *
 * Edge Cases:
 * - If the local video bin is already connected, a message is printed and the
 * function returns without making changes.
 * - If connecting the local video bin to the source bin fails, the program
 * exits.
 */
void RtmpStreamer::start_local_stream() {
    GstElement *bin =
        gst_bin_get_by_name(GST_BIN(pipeline), local_video_bin_name);
    if (bin) {
        gst_print("local bin already connected\n");
        g_object_unref(bin);
        return;
    }
    if (!connect_sink_bin_to_source_bin(source_bin, local_video_bin,
                                        &src_local_tee_pad, "tee",
                                        "local_video_src")) {
        exit(1);
    }
}

/**
 * @brief Stops the local video streaming by disconnecting the local video bin
 * from the source bin.
 *
 * The `stop_local_stream` function checks if the local video bin is currently
 * connected to the pipeline. If it is, the function disconnects the local video
 * bin from the source bin and updates internal state.
 *
 * Edge Cases:
 * - If the local video bin is already disconnected, a message is printed and
 * the function returns without making changes.
 * - If disconnecting the local video bin from the source bin fails, the program
 * exits.
 * - The `src_local_tee_pad` is set to `nullptr` after disconnection to avoid
 * dangling pointers.
 */
void RtmpStreamer::stop_local_stream() {
    GstElement *bin =
        gst_bin_get_by_name(GST_BIN(pipeline), local_video_bin_name);
    if (!bin) {
        gst_print("local bin already disconnected\n");
        return;
    }
    g_object_unref(bin);
    if (!disconnect_sink_bin_from_source_bin(source_bin, local_video_bin,
                                             src_local_tee_pad,
                                             "local_video_src")) {
        exit(1);
    }
    src_local_tee_pad = nullptr;
}

/**
 * @brief Disconnects a sink bin from a source bin and cleans up associated
 * elements.
 *
 * The `disconnect_sink_bin_from_source_bin` function removes the connection
 * between a sink bin and a source bin. It handles unlinking and removing pads,
 * releasing request pads from the `tee` element, and adjusting the pipeline
 * state.
 *
 * @param source_bin The source bin element from which the sink bin will be
 * disconnected.
 * @param sink_bin The sink bin element to be disconnected.
 * @param tee_pad The pad on the tee element used for the connection.
 * @param tee_ghost_pad_src_name The name of the ghost pad on the source bin
 * that needs to be removed.
 * @return `true` if the disconnection was successful, `false` otherwise.
 *
 * Edge Cases:
 * - If any of the input parameters (bins, pads, or tee element) are invalid or
 * NULL, an error message is printed and `false` is returned.
 * - If locking the pipeline state fails, an error message is printed and
 * `false` is returned.
 * - If unlinking the pads or removing the ghost pad fails, an error message is
 * printed and `false` is returned.
 * - If unlocking the pipeline state fails, an error message is printed and
 * `false` is returned.
 */
bool RtmpStreamer::disconnect_sink_bin_from_source_bin(
    GstElement *source_bin, GstElement *sink_bin, GstPad *tee_pad,
    const char *tee_ghost_pad_src_name) {
    GstPad *ghost_pad =
        gst_element_get_static_pad(source_bin, tee_ghost_pad_src_name);
    GstElement *tee = gst_bin_get_by_name(GST_BIN(source_bin), "tee");
    if (!source_bin) {
        g_printerr("Invalid bin\n");
        return false;
    }
    if (!tee) {
        g_printerr("Invalid tee element\n");
        return false;
    }
    if (!tee_pad) {
        g_printerr("Invalid tee pad\n");
        return false;
    }
    if (!ghost_pad) {
        g_printerr("Invalid source ghost pad\n");
        return false;
    }

    if (!source_bin || !tee || !tee_pad || !ghost_pad) {
        g_printerr("Invalid bin, tee, tee pad, or ghost pad\n");
        return false;
    }
    std::lock_guard<std::mutex> guard(handling_pipeline);

    // Set elements to NULL state
    // gst_element_set_state(pipeline, GST_STATE_NULL);

    // Lock the state of the elements before modifying the pipeline
    if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), TRUE)) {
        gst_printerr("unable to lock pipeline state\n");
        return false;
    }

    // Unlink the ghost pad from its peer
    GstPad *peer_pad = gst_pad_get_peer(ghost_pad);
    if (peer_pad) {
        gst_pad_unlink(ghost_pad, peer_pad);
        gst_object_unref(peer_pad);
    }

    // Remove the ghost pad from the bin
    gst_element_remove_pad(GST_ELEMENT(source_bin), ghost_pad);
    gst_object_unref(ghost_pad);

    // Unlink the tee pad from its peer
    peer_pad = gst_pad_get_peer(tee_pad);
    if (peer_pad) {
        gst_pad_unlink(tee_pad, peer_pad);
        gst_object_unref(peer_pad);
    }

    // Release the request pad from the tee element
    gst_element_release_request_pad(tee, tee_pad);

    gst_object_unref(tee_pad);

    gchar *name = gst_element_get_name(sink_bin);
    sink_bin = gst_bin_get_by_name(GST_BIN(pipeline), name);
    g_free(name);

    gst_bin_remove(GST_BIN(pipeline), sink_bin);

    // Unlock the mutex after making changes
    if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), FALSE)) {
        gst_printerr("unable to unlock locked pipeline state\n");
        return false;
    }

    // Set everyting but the disconnected Bin to PLAYING state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Clean up
    gst_object_unref(tee);

    return true;
}

/**
 * @brief Connects a sink bin to a source bin using a tee element and a ghost
 * pad.
 *
 * The `connect_sink_bin_to_source_bin` function sets up the necessary
 * connections to integrate a sink bin into a pipeline. It involves adding the
 * sink bin to the pipeline, requesting a pad from the tee element, and linking
 * the ghost pad of the source bin to the sink bin.
 *
 * @param source_bin The source bin element from which the connection will be
 * made.
 * @param sink_bin The sink bin element to be connected.
 * @param request_pad Pointer to a `GstPad` that will be set to the requested
 * pad from the tee element.
 * @param tee_element_name The name of the tee element used to request a pad.
 * @param tee_ghost_pad_name The name of the ghost pad on the source bin that
 * will be created.
 * @return `true` if the connection was successful, `false` otherwise.
 *
 * Edge Cases:
 * - If the source or sink bin is invalid, an error message is printed and
 * `false` is returned.
 * - If locking the pipeline state fails, an error message is printed and
 * `false` is returned.
 * - If adding the sink bin to the pipeline fails, an error message is printed
 * and the program exits.
 * - If the tee element cannot be found, an error message is printed and `false`
 * is returned.
 * - If linking the ghost pads fails, an error message is printed and `false` is
 * returned.
 * - If unlocking the pipeline state fails, an error message is printed and
 * `false` is returned.
 */
bool RtmpStreamer::connect_sink_bin_to_source_bin(
    GstElement *source_bin, GstElement *sink_bin, GstPad **request_pad,
    const char *tee_element_name, const char *tee_ghost_pad_name) {
    if (!source_bin || !sink_bin) {
        gst_printerr("Invalid source- or sink-bin, \n");
        return false;
    }

    std::lock_guard<std::mutex> guard(handling_pipeline);

    if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), TRUE)) {
        gst_printerr("unable to lock pipeline state\n");
        return false;
    }

    if (!gst_bin_add(GST_BIN(pipeline), sink_bin)) {
        gst_printerr("error, unable to add sink bin to pipeline!!\n");
        exit(1);
    }

    // Request Tee pad
    GstElement *tee =
        gst_bin_get_by_name(GST_BIN(source_bin), tee_element_name);
    if (!tee) {
        gst_printerr("No tee element by that name \n");
        if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), FALSE)) {
            gst_printerr("unable to unlock locked pipeline state\n");
            return false;
        }
        return false;
    }

    // Setup ghost pad for source bin
    *request_pad = gst_element_request_pad_simple(tee, "src_%u");
    gst_element_add_pad(source_bin,
                        gst_ghost_pad_new(tee_ghost_pad_name, *request_pad));

    // Link ghost pad of source_bin to sink_bin
    GstPad *src_ghost_pad =
        gst_element_get_static_pad(source_bin, tee_ghost_pad_name);
    GstPad *sink_ghost_pad = gst_element_get_static_pad(sink_bin, "sink");
    GstPadLinkReturn link_ok = gst_pad_link(src_ghost_pad, sink_ghost_pad);
    if (link_ok != GST_PAD_LINK_OK) {
        gst_printerr(
            "error linking source_bin ghost pad to sink_bin ghost pad. "
            "Error code: %d\n",
            link_ok);
        if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), FALSE)) {
            gst_printerr("unable to unlock locked pipeline state\n");
            return false;
        }
        return false;
    }

    // Unlock the state mutex after making changes
    if (!gst_element_set_locked_state(GST_ELEMENT(pipeline), FALSE)) {
        gst_printerr("unable to unlock locked pipeline state\n");
        return false;
    }

    gst_element_set_state(sink_bin, GST_STATE_PLAYING);

    // Unref objects
    gst_object_unref(src_ghost_pad);
    gst_object_unref(sink_ghost_pad);
    gst_object_unref(tee);

    return true;
}

void RtmpStreamer::debug_info() {
    const char *possible_states[5] = {"Void Pending", "Null", "Ready", "Paused",
                                      "Playing"};
    gst_println(
        "\n----------------- START DEBUG INFO "
        "-----------------------\n");
    gst_println("pipeline state: %s (pending state: %s)\n",
                possible_states[pipeline->current_state],
                possible_states[pipeline->pending_state]);
    gchar *name;
    GST_OBJECT_LOCK(pipeline);
    GList *pipeline_children = GST_BIN(pipeline)->children;

    while (pipeline_children) {
        GstBin *bin = GST_BIN(pipeline_children->data);
        gst_println("###### BIN: %s ######", gst_pad_get_name(bin));
        gst_println("bin state: %s (pending state: %s)\n",
                    possible_states[GST_ELEMENT(pipeline_children->data)
                                        ->current_state],
                    possible_states[GST_ELEMENT(pipeline_children->data)
                                        ->pending_state]);

        GList *pads = GST_ELEMENT(pipeline_children->data)->pads;
        gst_println("--- Bin Pads ---");
        while (pads) {
            GstPad *pad = GST_PAD(pads->data);
            name = gst_pad_get_name(pad);
            gst_println("bin pad: %s (is linked: %s)", name,
                        gst_pad_is_linked(pad) ? "true" : "false");
            g_free(name);
            pads = pads->next;
        }

        gst_println("\n--- Elements ---");
        GList *elements = bin->children;
        while (elements) {
            GstElement *element = GST_ELEMENT(elements->data);
            name = gst_element_get_name(element);
            gst_println("element: %s", name);
            g_free(name);
            gst_println("- element state: %s (pending state: %s)",
                        possible_states[element->current_state],
                        possible_states[element->pending_state]);

            GList *pads = element->pads;
            gst_println("element pads:");
            while (pads) {
                GstPad *pad = GST_PAD(pads->data);
                name = gst_pad_get_name(pad);
                gst_println("- element pad: %s (is linked: %s)", name,
                            gst_pad_is_linked(pad) ? "true" : "false");
                g_free(name);

                pads = pads->next;
            }
            elements = elements->next;
            gst_print("\n");
        }
        pipeline_children = pipeline_children->next;
        gst_print("\n");
    }
    GST_OBJECT_UNLOCK(pipeline);
    gst_println("----------------- END DEBUG INFO -----------------------\n");
}

bool RtmpStreamer::check_error() const {
    GstMessage *msg =
        gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR);
    if (msg != nullptr) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n",
                           GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n",
                           debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                return true;
            case GST_MESSAGE_EOS:
                //std::cout << "end" << std::endl;
                g_print("End-Of-Stream reached.\n");
                return true;
            default:
                g_print("Error: Unknown message type");
                exit(1);
                // We should not reach here because we only asked for ERRORs and
                // EOS
                break;
        }
        gst_message_unref(msg);
    }

    return false;
}
