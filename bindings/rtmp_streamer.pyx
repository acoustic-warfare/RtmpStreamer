# distutils: language = c++

from rtmp_streamer cimport RtmpStreamer
import ctypes
import numpy as np
cimport numpy as np

# It's necessary to call "import_array" if you use any part of the numpy PyArray_* API.
np.import_array()

RGB_BYTECOUNT = 3

cdef class PyRtmpStreamer:
    cdef RtmpStreamer *c_obj
    cdef unsigned int width
    cdef unsigned int height

    def __cinit__(self, width: int, height: int, rtmp_streaming_addr="rtmp://ome.waraps.org/app/unnamed"):
        self.width = width
        self.height = height
        self.c_obj = new RtmpStreamer(self.width, self.height, rtmp_streaming_addr)

    def __dealloc__(self):
        del self.c_obj

    def start_stream(self):
        self.c_obj.start_stream()

    def stop_stream(self):
        self.c_obj.stop_stream()

    def send_frame(self, frame: bytes) -> bool :
        cdef size_t c_size = self.width * self.height * RGB_BYTECOUNT
        cdef unsigned char * c_frame = <unsigned char *> frame
        return self.c_obj.send_frame(c_frame, c_size)

    def start_rtmp_stream(self):
        self.c_obj.start_rtmp_stream()

    def stop_rtmp_stream(self):
        self.c_obj.stop_rtmp_stream()

    def start_local_stream(self):
        self.c_obj.start_local_stream()

    def stop_local_stream(self):
        self.c_obj.stop_local_stream()

    def debug_info(self):
        self.c_obj.debug_info()
