add_library(LOG SHARED
    uni_log.c)

target_include_directories(LOG PUBLIC
	"../inc")

target_link_libraries(LOG HAL)