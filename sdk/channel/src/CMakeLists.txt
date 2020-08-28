add_library(CHANNEL SHARED
    uni_channel.c
    uni_communication.c)

target_include_directories(CHANNEL PUBLIC
	"../inc")

target_link_libraries(CHANNEL HAL LOG EVENT_LOOP LIST RINGBUF)