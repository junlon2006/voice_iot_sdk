add_library(RINGBUF SHARED
    uni_ringbuf.c)

target_include_directories(RINGBUF PUBLIC
	"../inc")