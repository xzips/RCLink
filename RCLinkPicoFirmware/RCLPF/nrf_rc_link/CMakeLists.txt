if (TARGET tinyusb_device)
    add_executable(nrf_rc_link
            nrf_rc_link.cpp
            )



    # pull in common dependencies
    target_link_libraries(nrf_rc_link
	PRIVATE	
	nrf24_driver 
	pico_stdlib
	hardware_spi
        hardware_gpio
	)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(nrf_rc_link 1)
    pico_enable_stdio_uart(nrf_rc_link 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(nrf_rc_link)

    # add url via pico_set_program_url
    example_auto_set_url(nrf_rc_link)
	
add_subdirectory(lib)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building nrf_rc_link because TinyUSB submodule is not initialized in the SDK")
endif()
