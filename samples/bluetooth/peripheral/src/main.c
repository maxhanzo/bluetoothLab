/* main.c - BLE Tutorial peripheral */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/printk.h>
#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/conn.h>
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
/* Device Information Service                                                 */
/* -------------------------------------------------------------------------- */

#define SERIAL_RAW_LEN 16
#define SERIAL_STR_LEN (SERIAL_RAW_LEN * 2 + 1)

static char serial_number[SERIAL_STR_LEN];
static const char hardware_revision[] = "NICE-NANO-COMPATIBLE";
static const char firmware_revision[] = "0.5.1";

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
		conn, attr, buf, len, offset,
		serial_number, strlen(serial_number)
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
		conn, attr, buf, len, offset,
		hardware_revision, strlen(hardware_revision)
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
		conn, attr, buf, len, offset,
		firmware_revision, strlen(firmware_revision)
	);
}

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

#define BT_UUID_TUTORIAL_OBSERVABLE_WRITE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000004ULL)

#define BT_UUID_TUTORIAL_OBSERVABLE_VALUE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000005ULL)


#define BT_UUID_TUTORIAL_WRITE_NOTIFY_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000006ULL)

#define BT_UUID_TUTORIAL_WRITE_NO_RSP_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000007ULL)

#define BT_UUID_TUTORIAL_WRITE_NO_RSP_READ_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000008ULL)


#define BT_UUID_TUTORIAL_SECURE_WRITE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x000000000009ULL)

#define BT_UUID_TUTORIAL_SECURE_STATE_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x00000000000AULL)

static const struct bt_uuid_128 tutorial_service_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_SERVICE_VAL);

static const struct bt_uuid_128 tutorial_write_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_WRITE_VAL);

static const struct bt_uuid_128 tutorial_last_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_LAST_VAL);

static const struct bt_uuid_128 tutorial_observable_write_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_OBSERVABLE_WRITE_VAL);

static const struct bt_uuid_128 tutorial_observable_value_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_OBSERVABLE_VALUE_VAL);


static const struct bt_uuid_128 tutorial_write_notify_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_WRITE_NOTIFY_VAL);

static const struct bt_uuid_128 tutorial_write_no_rsp_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_WRITE_NO_RSP_VAL);

static const struct bt_uuid_128 tutorial_write_no_rsp_read_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_WRITE_NO_RSP_READ_VAL);


static const struct bt_uuid_128 tutorial_secure_write_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_SECURE_WRITE_VAL);

static const struct bt_uuid_128 tutorial_secure_state_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_SECURE_STATE_VAL);

/* Simple WRITE -> READ-back pair: ...0002 and ...0003 */

#define LAST_VALUE_MAX_LEN 64

static uint8_t last_value[LAST_VALUE_MAX_LEN];
static uint16_t last_value_len;
static bool has_last_value;

static const struct bt_gatt_attr *periodic_notify_attr;

static bool periodic_notify_enabled;
static uint8_t periodic_notify_counter;

static void periodic_notify_work_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(
	periodic_notify_work,
	periodic_notify_work_handler
);

static void periodic_notify_work_handler(struct k_work *work)
{
	int err;

	if (!periodic_notify_enabled || periodic_notify_attr == NULL) {
		return;
	}

	/*
	 * This characteristic represents an event stream rather than persistent
	 * state. Every two seconds a new one-byte event is generated.
	 *
	 * The counter naturally wraps from 0xFF back to 0x00.
	 */
	periodic_notify_counter++;

	err = bt_gatt_notify(
		NULL,
		periodic_notify_attr,
		&periodic_notify_counter,
		sizeof(periodic_notify_counter)
	);

	if (err == 0) {
		printk(
			"Periodic event notification sent: %02X\n",
			periodic_notify_counter
		);
	} else {
		printk(
			"Periodic event notification failed: %d\n",
			err
		);
	}

	if (periodic_notify_enabled) {
		k_work_reschedule(
			&periodic_notify_work,
			K_SECONDS(2)
		);
	}
}

static void write_notify_ccc_changed(
	const struct bt_gatt_attr *attr,
	uint16_t value)
{
	periodic_notify_enabled = (value == BT_GATT_CCC_NOTIFY);

	if (periodic_notify_enabled) {
		/*
		 * Start a fresh sequence for each subscription.
		 * First notification will be 0x01 after two seconds.
		 */
		periodic_notify_counter = 0U;

		printk("Periodic event notifications enabled\n");

		k_work_reschedule(
			&periodic_notify_work,
			K_SECONDS(2)
		);
	} else {
		printk("Periodic event notifications disabled\n");
		k_work_cancel_delayable(&periodic_notify_work);
	}
}

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
		printk("%02X%s", last_value[i], (i + 1U < len) ? " " : "");
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
			conn, attr, buf, len, offset,
			not_available, sizeof(not_available) - 1U
		);
	}

	return bt_gatt_attr_read(
		conn, attr, buf, len, offset,
		last_value, last_value_len
	);
}

/* WRITE -> READ | NOTIFY pair: ...0004 and ...0005 */

#define OBSERVABLE_VALUE_MAX_LEN 64

static uint8_t observable_value[OBSERVABLE_VALUE_MAX_LEN];
static uint16_t observable_value_len;
static bool has_observable_value;
static const struct bt_gatt_attr *observable_value_attr;

static ssize_t read_observable_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	static const char not_available[] = "N/A";

	if (!has_observable_value) {
		return bt_gatt_attr_read(
			conn, attr, buf, len, offset,
			not_available, sizeof(not_available) - 1U
		);
	}

	return bt_gatt_attr_read(
		conn, attr, buf, len, offset,
		observable_value, observable_value_len
	);
}

static void observable_ccc_changed(
	const struct bt_gatt_attr *attr,
	uint16_t value)
{
	printk(
		"Observable notifications %s\n",
		value == BT_GATT_CCC_NOTIFY ? "enabled" : "disabled"
	);
}

static ssize_t write_observable_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags)
{
	int err;

	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len > OBSERVABLE_VALUE_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(observable_value, buf, len);
	observable_value_len = len;
	has_observable_value = true;

	printk("Observable Write received %u byte(s): ", len);

	for (uint16_t i = 0; i < len; i++) {
		printk("%02X%s", observable_value[i],
		       (i + 1U < len) ? " " : "");
	}

	printk("\n");

	/*
	 * If the connected client subscribed to ...0005, Zephyr sends the
	 * notification. If nobody subscribed, bt_gatt_notify() returns an error,
	 * but the new value remains stored and is still available through READ.
	 */
	if (observable_value_attr != NULL) {
		err = bt_gatt_notify(
			conn,
			observable_value_attr,
			observable_value,
			observable_value_len
		);

		if (err == 0) {
			printk("Observable notification sent\n");
		} else {
			printk("Observable notification not sent (err %d)\n", err);
		}
	}

	return len;
}


/* WRITE WITHOUT RESPONSE -> READ pair: ...0007 and ...0008 */

#define WRITE_NO_RSP_MAX_LEN 64

static uint8_t write_no_rsp_value[WRITE_NO_RSP_MAX_LEN];
static uint16_t write_no_rsp_value_len;
static bool has_write_no_rsp_value;

static ssize_t write_without_response_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags)
{
	if (!(flags & BT_GATT_WRITE_FLAG_CMD)) {
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
	}

	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len > WRITE_NO_RSP_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(write_no_rsp_value, buf, len);
	write_no_rsp_value_len = len;
	has_write_no_rsp_value = true;

	printk("Write Without Response received %u byte(s): ", len);

	for (uint16_t i = 0; i < len; i++) {
		printk("%02X%s", write_no_rsp_value[i],
		       (i + 1U < len) ? " " : "");
	}

	printk("\n");

	return len;
}

static ssize_t read_write_without_response_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	static const char not_available[] = "N/A";

	if (!has_write_no_rsp_value) {
		return bt_gatt_attr_read(
			conn, attr, buf, len, offset,
			not_available, sizeof(not_available) - 1U
		);
	}

	return bt_gatt_attr_read(
		conn, attr, buf, len, offset,
		write_no_rsp_value, write_no_rsp_value_len
	);
}


/* SECURE WRITE -> SECURE READ | NOTIFY pair: ...0009 and ...000A */

#define SECURE_VALUE_MAX_LEN 64

static uint8_t secure_value[SECURE_VALUE_MAX_LEN];
static uint16_t secure_value_len;
static bool has_secure_value;
static const struct bt_gatt_attr *secure_state_attr;

static ssize_t read_secure_state(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	void *buf,
	uint16_t len,
	uint16_t offset)
{
	static const char not_available[] = "N/A";

	if (!has_secure_value) {
		return bt_gatt_attr_read(
			conn, attr, buf, len, offset,
			not_available, sizeof(not_available) - 1U
		);
	}

	return bt_gatt_attr_read(
		conn, attr, buf, len, offset,
		secure_value, secure_value_len
	);
}

static void secure_ccc_changed(
	const struct bt_gatt_attr *attr,
	uint16_t value)
{
	printk(
		"Secure notifications %s\n",
		value == BT_GATT_CCC_NOTIFY ? "enabled" : "disabled"
	);
}

static ssize_t write_secure_value(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags)
{
	int err;

	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len > SECURE_VALUE_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(secure_value, buf, len);
	secure_value_len = len;
	has_secure_value = true;

	printk("Secure Write received %u byte(s): ", len);

	for (uint16_t i = 0; i < len; i++) {
		printk("%02X%s", secure_value[i],
		       (i + 1U < len) ? " " : "");
	}

	printk("\n");

	if (secure_state_attr != NULL) {
		err = bt_gatt_notify(
			conn,
			secure_state_attr,
			secure_value,
			secure_value_len
		);

		if (err == 0) {
			printk("Secure notification sent\n");
		} else {
			printk("Secure notification not sent (err %d)\n", err);
		}
	}

	return len;
}

static int start_advertising(void);

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err != 0U) {
		printk("Connection failed (err 0x%02X)\n", err);
		return;
	}

	printk("Connected. Security level: %u\n", bt_conn_get_security(conn));
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02X)\n", reason);
	printk("Waiting for connection object to be recycled before advertising again...\n");
}

static void recycled(void)
{
	int err;

	printk("Connection recycled. Restarting advertising...\n");

	err = start_advertising();
	if (err != 0) {
		printk("Advertising restart failed: %d\n", err);
	}
}

static void security_changed(
	struct bt_conn *conn,
	bt_security_t level,
	enum bt_security_err err)
{
	if (err == BT_SECURITY_ERR_SUCCESS) {
		printk("Security changed. New level: %u\n", level);
	} else {
		printk("Security change failed (level %u, err %d)\n", level, err);
	}
}


static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	printk("Pairing complete. Bonded: %s\n", bonded ? "yes" : "no");
}

static void pairing_failed(
	struct bt_conn *conn,
	enum bt_security_err reason)
{
	printk("Pairing failed (reason %d)\n", reason);
}

static void bond_deleted(uint8_t id, const bt_addr_le_t *peer)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(peer, addr, sizeof(addr));
	printk("Bond deleted: %s (local id %u)\n", addr, id);
}

static struct bt_conn_auth_info_cb auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed,
	.bond_deleted = bond_deleted,
};

static void print_bond(
	const struct bt_bond_info *info,
	void *user_data)
{
	int *count = user_data;
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(&info->addr, addr, sizeof(addr));

	(*count)++;
	printk("  Bond %d: %s\n", *count, addr);
}

static void print_stored_bonds(void)
{
	int count = 0;

	printk("Stored bonds:\n");
	bt_foreach_bond(BT_ID_DEFAULT, print_bond, &count);

	if (count == 0) {
		printk("  (none)\n");
	} else {
		printk("Total stored bonds: %d\n", count);
	}
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
	.recycled = recycled,
};

BT_GATT_SERVICE_DEFINE(tutorial_svc,
	BT_GATT_PRIMARY_SERVICE(&tutorial_service_uuid),

	/* ...0002: WRITE */
	BT_GATT_CHARACTERISTIC(
		&tutorial_write_uuid.uuid,
		BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_WRITE,
		NULL,
		write_value,
		NULL
	),

	/* ...0003: READ */
	BT_GATT_CHARACTERISTIC(
		&tutorial_last_uuid.uuid,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_last_value,
		NULL,
		NULL
	),

	/* ...0004: WRITE */
	BT_GATT_CHARACTERISTIC(
		&tutorial_observable_write_uuid.uuid,
		BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_WRITE,
		NULL,
		write_observable_value,
		NULL
	),

	/* ...0005: READ | NOTIFY */
	BT_GATT_CHARACTERISTIC(
		&tutorial_observable_value_uuid.uuid,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_observable_value,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		observable_ccc_changed,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
	),

	/* ...0006: NOTIFY-only periodic event stream (one byte every 2 seconds) */
	BT_GATT_CHARACTERISTIC(
		&tutorial_write_notify_uuid.uuid,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		write_notify_ccc_changed,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
	),

	/* ...0007: WRITE WITHOUT RESPONSE */
	BT_GATT_CHARACTERISTIC(
		&tutorial_write_no_rsp_uuid.uuid,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE,
		NULL,
		write_without_response_value,
		NULL
	),

	/* ...0008: READ last value received by ...0007 */
	BT_GATT_CHARACTERISTIC(
		&tutorial_write_no_rsp_read_uuid.uuid,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ,
		read_write_without_response_value,
		NULL,
		NULL
	),

	/* ...0009: SECURE WRITE - requires an encrypted connection */
	BT_GATT_CHARACTERISTIC(
		&tutorial_secure_write_uuid.uuid,
		BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_WRITE_ENCRYPT,
		NULL,
		write_secure_value,
		NULL
	),

	/* ...000A: SECURE READ | NOTIFY - requires an encrypted connection */
	BT_GATT_CHARACTERISTIC(
		&tutorial_secure_state_uuid.uuid,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ_ENCRYPT,
		read_secure_state,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		secure_ccc_changed,
		BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT
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

static int start_advertising(void)
{
	int err;

	err = bt_le_adv_start(
		BT_LE_ADV_CONN_FAST_1,
		ad,
		ARRAY_SIZE(ad),
		sd,
		ARRAY_SIZE(sd)
	);

	if (err != 0) {
		printk("Advertising failed: %d\n", err);
		return err;
	}

	printk("Advertising started. Look for: %s\n", CONFIG_BT_DEVICE_NAME);
	return 0;
}


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

	/*
	 * Attribute layout inside tutorial_svc:
	 * 0   Primary service
	 * 1   Characteristic declaration ...0002
	 * 2   Characteristic value       ...0002
	 * 3   Characteristic declaration ...0003
	 * 4   Characteristic value       ...0003
	 * 5   Characteristic declaration ...0004
	 * 6   Characteristic value       ...0004
	 * 7   Characteristic declaration ...0005
	 * 8   Characteristic value       ...0005
	 * 9   CCCD                       ...0005
	 * 10  Characteristic declaration ...0006
	 * 11  Characteristic value       ...0006
	 * 12  CCCD                       ...0006
	 * 13  Characteristic declaration ...0007
	 * 14  Characteristic value       ...0007
	 * 15  Characteristic declaration ...0008
	 * 16  Characteristic value       ...0008
	 * 17  Characteristic declaration ...0009
	 * 18  Characteristic value       ...0009
	 * 19  Characteristic declaration ...000A
	 * 20  Characteristic value       ...000A
	 * 21  CCCD                       ...000A
	 */
	observable_value_attr = &tutorial_svc.attrs[8];
	periodic_notify_attr = &tutorial_svc.attrs[11];
	secure_state_attr = &tutorial_svc.attrs[20];

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

	printk("Loading stored Bluetooth settings...\n");

	err = settings_load();
	if (err != 0) {
		printk("settings_load() failed: %d\n", err);
		return 0;
	}

	printk("Bluetooth settings loaded successfully.\n");

	err = bt_conn_auth_info_cb_register(&auth_info_callbacks);
	if (err != 0) {
		printk("Auth info callback registration failed: %d\n", err);
		return 0;
	}

	print_stored_bonds();

	printk("Starting advertising...\n");

	err = start_advertising();
	if (err != 0) {
		while (1) {
			gpio_pin_toggle_dt(&status_led);
			k_sleep(K_MSEC(200));
		}
	}

	gpio_pin_set_dt(&status_led, 1);

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
