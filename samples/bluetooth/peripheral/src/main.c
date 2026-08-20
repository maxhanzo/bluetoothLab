/* main.c - BLE Tutorial peripheral */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LED_NODE DT_ALIAS(led0)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "This board does not define a usable led0 alias"
#endif

static const struct gpio_dt_spec status_led =
	GPIO_DT_SPEC_GET(LED_NODE, gpios);

/* -------------------------------------------------------------------------- */
/* Device identity                                                            */
/* -------------------------------------------------------------------------- */

#define SERIAL_RAW_LEN 16
#define SERIAL_STR_LEN (SERIAL_RAW_LEN * 2 + 1)

static char serial_number[SERIAL_STR_LEN];
static const char hardware_revision[] = "NICE-NANO-COMPATIBLE";
static const char firmware_revision[] = "0.1.0";

static void init_serial_number(void)
{
	uint8_t id[SERIAL_RAW_LEN];
	ssize_t len;

	len = hwinfo_get_device_id(id, sizeof(id));

	if (len <= 0) {
		strcpy(serial_number, "UNKNOWN");
		return;
	}

	for (ssize_t i = 0; i < len; i++) {
		snprintf(
			&serial_number[i * 2],
			sizeof(serial_number) - (i * 2),
			"%02X",
			id[i]
		);
	}
}

static ssize_t read_serial_number(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		serial_number,
		strlen(serial_number)
	);
}

static ssize_t read_hardware_revision(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		hardware_revision,
		strlen(hardware_revision)
	);
}

static ssize_t read_firmware_revision(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		firmware_revision,
		strlen(firmware_revision)
	);
}

/* Standard Bluetooth Device Information Service (0x180A). */
BT_GATT_SERVICE_DEFINE(dis_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_DIS),

	BT_GATT_CHARACTERISTIC(
		BT_UUID_DIS_SERIAL_NUMBER,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_serial_number,
		NULL,
		NULL
	),

	BT_GATT_CHARACTERISTIC(
		BT_UUID_DIS_HARDWARE_REVISION,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_hardware_revision,
		NULL,
		NULL
	),

	BT_GATT_CHARACTERISTIC(
		BT_UUID_DIS_FIRMWARE_REVISION,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_firmware_revision,
		NULL,
		NULL
	)
);

/* -------------------------------------------------------------------------- */
/* BLE Tutorial custom service                                                */
/* -------------------------------------------------------------------------- */

#define BT_UUID_TUTORIAL_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000001ULL)

#define BT_UUID_TUTORIAL_WRITE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000002ULL)

#define BT_UUID_TUTORIAL_LAST_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000003ULL)

static const struct bt_uuid_128 tutorial_service_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_SERVICE_VAL);

static const struct bt_uuid_128 tutorial_write_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_WRITE_VAL);

static const struct bt_uuid_128 tutorial_last_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_LAST_VAL);

#define LAST_VALUE_MAX_LEN 64

static uint8_t last_value[LAST_VALUE_MAX_LEN];
static uint16_t last_value_len;
static bool has_last_value;

static ssize_t write_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags)
{
	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len > LAST_VALUE_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(last_value, buf, len);
	last_value_len = len;
	has_last_value = true;

	printk("Write Value received %u byte(s): ", len);

	for (uint16_t i = 0; i < len; i++) {
		printk("%02X", last_value[i]);

		if (i + 1U < len) {
			printk(" ");
		}
	}

	printk("\n");

	return len;
}

static ssize_t read_last_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	static const char not_available[] = "N/A";

	if (!has_last_value) {
		return bt_gatt_attr_read(
			conn,
			attr,
			buf,
			len,
			offset,
			not_available,
			sizeof(not_available) - 1U
		);
	}

	return bt_gatt_attr_read(
		conn,
		attr,
		buf,
		len,
		offset,
		last_value,
		last_value_len
	);
}

BT_GATT_SERVICE_DEFINE(tutorial_svc,
	BT_GATT_PRIMARY_SERVICE(&tutorial_service_uuid),

	BT_GATT_CHARACTERISTIC(
		&tutorial_write_uuid.uuid,
		BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_WRITE,
		NULL,
		write_value,
		NULL
	),

	BT_GATT_CHARACTERISTIC(
		&tutorial_last_uuid.uuid,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_last_value,
		NULL,
		NULL
	)
);

/* -------------------------------------------------------------------------- */
/* Advertising                                                                */
/* -------------------------------------------------------------------------- */

static const struct bt_data ad[] = {
	BT_DATA_BYTES(
		BT_DATA_FLAGS,
		BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR
	),
};

static const struct bt_data sd[] = {
	BT_DATA(
		BT_DATA_NAME_COMPLETE,
		CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1
	),
};

/* -------------------------------------------------------------------------- */
/* Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
	int err;

	if (!gpio_is_ready_dt(&status_led)) {
		return 0;
	}

	err = gpio_pin_configure_dt(
		&status_led,
		GPIO_OUTPUT_INACTIVE
	);

	if (err != 0) {
		return 0;
	}

	init_serial_number();

	printk("\nBLE Tutorial starting...\n");
	printk("Serial Number: %s\n", serial_number);
	printk("Hardware Revision: %s\n", hardware_revision);
	printk("Firmware Revision: %s\n", firmware_revision);

	printk("Calling bt_enable()...\n");

	err = bt_enable(NULL);

	if (err != 0) {
		printk("Bluetooth initialization failed: %d\n", err);

		while (1) {
			gpio_pin_toggle_dt(&status_led);
			k_sleep(K_SECONDS(1));
		}
	}

	printk("Bluetooth initialized successfully.\n");
	printk("Starting advertising...\n");

	err = bt_le_adv_start(
		BT_LE_ADV_CONN_FAST_1,
		ad,
		ARRAY_SIZE(ad),
		sd,
		ARRAY_SIZE(sd)
	);

	if (err != 0) {
		printk("Advertising failed: %d\n", err);

		while (1) {
			gpio_pin_toggle_dt(&status_led);
			k_sleep(K_MSEC(200));
		}
	}

	printk("Advertising successfully started.\n");
	printk("Look for: %s\n", CONFIG_BT_DEVICE_NAME);

	gpio_pin_set_dt(&status_led, 1);

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
