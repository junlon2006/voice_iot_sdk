SET(CMAKE_SYSTEM_NAME Linux)
option(X86    "Enable x86 platform cross compile" OFF)

if(X86)
SET(CMAKE_C_COMPILER gcc)
message("x86 platform")
endif()