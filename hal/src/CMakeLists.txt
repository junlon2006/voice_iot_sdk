add_library(HAL SHARED
    porting.c)

target_include_directories(HAL PUBLIC
	"../inc")

target_link_libraries(HAL pthread)