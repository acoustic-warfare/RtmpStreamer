
ctypedef unsigned int uint
ctypedef unsigned long int uint64_t

# cdef extern from "src/rtmp.cpp":
#     pass

# cdef extern from "opencv2/core/core.hpp" namespace "cv":
#     cdef cppclass Mat:
#         Mat()
#         Mat(const Mat &m)
#         # Add more OpenCV methods and constructors as needed
#
# cdef extern from "opencv2/core.hpp":
#     cdef cppclass Error:
#         # You may need to adjust this based on the actual OpenCV API.
#         # For instance, if OpenCV uses error codes or exceptions:
#         Error()
#         Error(int code, const std::string &msg)
#         int code
#         std::string msg
#
#     # Define common error codes (if they exist)
#     cdef cppclass ErrorCode:
#         # Error codes for common OpenCV errors
#         # You need to check OpenCV documentation for actual codes.
#         cdef const int StsOk = 0
#         cdef const int StsError = 1
#         cdef const int StsBadArg = 2
#         cdef const int StsNoMem = 3
#         # Add other relevant error codes

cdef extern from "rtmp.hpp":
    cdef cppclass RtmpStreamer:
        RtmpStreamer() except +
        RtmpStreamer(uint width, unsigned int height) except +
        void start_stream()
        void stop_stream()
        #bint send_frame(const Mat &frame)
        bint send_frame(unsigned char *frame, uint64_t size)
        void start_rtmp_stream()
        void stop_rtmp_stream()
        void start_local_stream()
        void stop_local_stream()
