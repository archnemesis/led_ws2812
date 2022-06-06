#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "rbgled_service.h"

static void on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t on_receive(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);
static ssize_t on_receive_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

BT_GATT_SERVICE_DEFINE(rgbled_service,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_RGBLED),
    BT_GATT_CHARACTERISTIC(BT_UUID_RGBLED_CMD,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
        NULL, on_receive, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_RGBLED_DATA,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
        NULL, on_receive_data, NULL),
    BT_GATT_CCC(on_cccd_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

#define RGBLED_MAX_TRANSMIT_SIZE    240

int rgbled_service_init()
{
    int err = 0;

    return err;
}

static void on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    ARG_UNUSED(value);
}

/**
 * @brief This callback receives command data.
 * 
 * @param conn 
 * @param attr 
 * @param buf 
 * @param len 
 * @param offset 
 * @param flags 
 * @return ssize_t 
 */
static ssize_t on_receive(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    return len;
}

/**
 * @brief This callback receives RGB data.
 * 
 * @param conn 
 * @param attr 
 * @param buf 
 * @param len 
 * @param offset 
 * @param flags 
 * @return ssize_t 
 */
static ssize_t on_receive_data(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    return len;
}