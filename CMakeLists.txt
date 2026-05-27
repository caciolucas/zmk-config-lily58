if(CONFIG_ZMK_KEYSTATUS_BLE)
    target_sources(app PRIVATE src/keystatus_ble.c)
endif()
