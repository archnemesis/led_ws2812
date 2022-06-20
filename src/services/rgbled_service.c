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
#include "rgbled.h"

#define COMMAND_SET_MODE    0x01
#define COMMAND_SET_PATTERN 0x02
#define COMMAND_SET_COLOR   0x03

struct command_t {
    uint8_t opcode;
    uint8_t param1;
    uint8_t param2;
    uint8_t param3;
    uint8_t param4;
};

static void on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t on_receive(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);
static ssize_t on_receive_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

BT_GATT_SERVICE_DEFINE(rgbled_service,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_RGBLED),
    BT_GATT_CHARACTERISTIC(BT_UUID_RGBLED_CMD,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        NULL, on_receive, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_RGBLED_DATA,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        NULL, on_receive_data, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_RGBLED_COLORS,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        NULL, NULL, NULL),
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

static void rgbled_update_colors(struct bt_conn *conn, const struct bt_gatt_attr *chrc)
{
    // struct bt_gatt_write_params params;
    // params.handle = &rgbled_service.attrs[2];
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
    printk("Got data from characteristic");

    if (len <= sizeof(struct command_t)) {
        struct command_t command;
        memcpy((void *)&command, buf, len);

        switch (command.opcode) {
            case COMMAND_SET_PATTERN:    
                rgbled_set_pattern(command.param1);
                break;
            case COMMAND_SET_COLOR:
            {
                int index = command.param1;
                uint8_t r = command.param2;
                uint8_t g = command.param3;
                uint8_t b = command.param4;

                rgbled_set_color(index, (struct led_rgb)RGB(r,g,b));

                break;
            }
        }
    }

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