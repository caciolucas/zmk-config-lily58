if(CONFIG_ZMK_KEYSTATUS_BLE)
    # zmk_keymap_* and zmk_ble_* APIs only exist on the central (left) side
    if(CONFIG_ZMK_SPLIT_ROLE_CENTRAL OR NOT CONFIG_ZMK_SPLIT)
        target_sources(app PRIVATE src/keystatus_ble.c)
    endif()
endif()
