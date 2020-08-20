add_library(EVENT_LOOP SHARED
    uni_event_list.c)

target_include_directories(EVENT_LOOP PUBLIC
	"../inc")

target_link_libraries(EVENT_LOOP HAL LOG LIST)