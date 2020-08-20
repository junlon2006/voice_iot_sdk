add_executable(APP
    app.c
    main.c
    uni_uart.c)

target_include_directories(APP PUBLIC
    "../inc")

target_link_libraries(APP HAL EVENT_LOOP LOG CHANNEL RINGBUF)