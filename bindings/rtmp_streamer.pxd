
ctypedef unsigned int uint
ctypedef unsigned long int uint64_t

cdef extern from "rtmp.hpp":
    cdef cppclass RtmpStreamer:
        RtmpStreamer() except +
        RtmpStreamer(uint width, uint height) except +
        void start_stream()
        void stop_stream()
        bint send_frame(unsigned char *frame, uint64_t size)
        void start_rtmp_stream()
        void stop_rtmp_stream()
        void start_local_stream()
        void stop_local_stream()
