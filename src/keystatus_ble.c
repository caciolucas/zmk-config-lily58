/*
 * Exposes current layer index and BT profile index over a custom BLE GATT service.
 *
 * Service UUID:      CAFE0001-CAFE-CAFE-CAFE-CAFECAFECAFE
 * Layer char UUID:   CAFE0002-CAFE-CAFE-CAFE-CAFECAFECAFE  (read + notify, uint8)
 * Profile char UUID: CAFE0003-CAFE-CAFE-CAFE-CAFECAFECAFE  (read + notify, uint8, 0-based)
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/ble.h>
#endif

static struct bt_uuid_128 ks_svc_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xcafe0001U, 0xcafe, 0xcafe, 0xcafe, 0xcafecafecafeULL));
static struct bt_uuid_128 ks_layer_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xcafe0002U, 0xcafe, 0xcafe, 0xcafe, 0xcafecafecafeULL));
static struct bt_uuid_128 ks_profile_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xcafe0003U, 0xcafe, 0xcafe, 0xcafe, 0xcafecafecafeULL));

static uint8_t layer_val;
static uint8_t profile_val;

static ssize_t read_u8(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(uint8_t));
}

/* attrs[0]=service, [1]=layer chrc decl, [2]=layer value, [3]=layer CCC,
 *                   [4]=profile chrc decl, [5]=profile value, [6]=profile CCC */
BT_GATT_SERVICE_DEFINE(keystatus_svc,
    BT_GATT_PRIMARY_SERVICE(&ks_svc_uuid),
    BT_GATT_CHARACTERISTIC(&ks_layer_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ, read_u8, NULL, &layer_val),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&ks_profile_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ, read_u8, NULL, &profile_val),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static int on_layer_changed(const zmk_event_t *eh) {
    uint8_t highest = zmk_keymap_highest_layer_active();
    if (highest != layer_val) {
        layer_val = highest;
        bt_gatt_notify(NULL, &keystatus_svc.attrs[2], &layer_val, sizeof(layer_val));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keystatus_layer, on_layer_changed);
ZMK_SUBSCRIPTION(keystatus_layer, zmk_layer_state_changed);

#if IS_ENABLED(CONFIG_ZMK_BLE)
static int on_profile_changed(const zmk_event_t *eh) {
    uint8_t idx = (uint8_t)zmk_ble_active_profile_index();
    if (idx != profile_val) {
        profile_val = idx;
        bt_gatt_notify(NULL, &keystatus_svc.attrs[5], &profile_val, sizeof(profile_val));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keystatus_profile, on_profile_changed);
ZMK_SUBSCRIPTION(keystatus_profile, zmk_ble_active_profile_changed);
#endif
