# distutils: language = c++

from rtmp_streamer cimport RtmpStreamer
#from rtmp_streamer cimport Mat, Error

RGB_BYTECOUNT = 3

cdef class PyRtmpStreamer:
    cdef RtmpStreamer *c_obj

    def __cinit__(self, width=1024, height=1024):
        self.c_obj = new RtmpStreamer(width, height)

    def __dealloc__(self):
        del self.c_obj

    def start_stream(self):
        self.c_obj.start_stream()

    def stop_stream(self):
        self.c_obj.stop_stream()

    def send_frame(self, frame: np.bytes, width: int, height: int) -> bool :
        pass
        # cdef size_t c_size = width * height * RGB_BYTECOUNT
        # cdef unsigned char *c_frame = <unsigned char *> frame
        # return self.c_obj.send_frame(c_frame, c_size)

    def start_rtmp_stream(self):
        self.c_obj.start_rtmp_stream()

    def stop_rtmp_stream(self):
        self.c_obj.stop_rtmp_stream()

    def start_local_stream(self):
        self.c_obj.start_local_stream()

    def stop_local_stream(self):
        self.c_obj.stop_local_stream()
