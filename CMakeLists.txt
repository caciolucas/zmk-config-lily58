if(CONFIG_ZMK_KEYSTATUS_BLE AND NOT CONFIG_ZMK_SPLIT_BLE_ROLE_PERIPHERAL)
    target_sources(app PRIVATE src/keystatus_ble.c)
endif()
