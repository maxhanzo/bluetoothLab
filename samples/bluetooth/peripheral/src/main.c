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
static const char firmware_revision[] = "0.6.2";

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


#define BT_UUID_TUTORIAL_FILE_TRANSFER_RX_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x00000000000BULL)

#define BT_UUID_TUTORIAL_FILE_TRANSFER_TX_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x00000000000CULL)


#define BT_UUID_TUTORIAL_FILE_COUNT_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x00000000000DULL)

#define BT_UUID_TUTORIAL_TOTAL_UPLOADED_BYTES_VAL \
	BT_UUID_128_ENCODE(0x7e57a000, 0x0000, 0x4b1a, 0x9c00, 0x00000000000EULL)

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


static const struct bt_uuid_128 tutorial_file_transfer_rx_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_FILE_TRANSFER_RX_VAL);

static const struct bt_uuid_128 tutorial_file_transfer_tx_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_FILE_TRANSFER_TX_VAL);


static const struct bt_uuid_128 tutorial_file_count_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_FILE_COUNT_VAL);

static const struct bt_uuid_128 tutorial_total_uploaded_bytes_uuid =
	BT_UUID_INIT_128(BT_UUID_TUTORIAL_TOTAL_UPLOADED_BYTES_VAL);

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

/* File-transfer declarations needed by connection callbacks */
enum ftr_state {
	FTR_STATE_IDLE = 0,
	FTR_STATE_UPLOAD,
	FTR_STATE_DOWNLOAD_WAIT_ACK,
	FTR_STATE_DOWNLOAD_SENDING,
};

static enum ftr_state ftr_state = FTR_STATE_IDLE;
static bool ftr_tx_notify_enabled;
static bool file_count_notify_enabled;

static void ftr_abort(void);
static void notify_file_count(void);

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

	if (ftr_state != FTR_STATE_IDLE) {
		ftr_abort();
	}

	ftr_tx_notify_enabled = false;
	file_count_notify_enabled = false;

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


/* -------------------------------------------------------------------------- */
/* File transfer demo: ...000B RX (WRITE WITHOUT RESPONSE), ...000C TX NOTIFY */
/* -------------------------------------------------------------------------- */

/*
 * This intentionally follows the same shape as the GameGolf transport:
 *
 * RX  = WRITE WITHOUT RESPONSE
 * TX  = NOTIFY
 *
 * Wire format:
 *
 *   Request:
 *     'U' [optional uint32_le expected size]   start upload to peripheral
 *     'D'                                      download last uploaded file
 *
 *   Data chunk:
 *     byte 0 = 0x02 (STR) normal chunk
 *              0x03 (ETX) final chunk
 *     byte 1 = sequence number (uint8, wraps naturally)
 *     byte 2.. = payload
 *
 *   Control:
 *     0x06 seq = ACK
 *     0x15 seq = NACK
 *     0x18     = CAN / abort
 *
 * As in the GameGolf protocol, one ACK is exchanged after each batch of
 * 8 chunks and after the final ETX chunk.
 */

#define FTR_ASCII_STR   0x02
#define FTR_ASCII_ETX   0x03
#define FTR_ASCII_ACK   0x06
#define FTR_ASCII_NACK  0x15
#define FTR_ASCII_CAN   0x18

#define FTR_DIRECTION_DOWNLOAD 'D'
#define FTR_DIRECTION_UPLOAD   'U'

#define FTR_CHUNK_SIZE          20U
#define FTR_HEADER_SIZE         2U
#define FTR_PAYLOAD_SIZE        (FTR_CHUNK_SIZE - FTR_HEADER_SIZE)
#define FTR_CHUNKS_PER_ACK      8U

/*
 * RAM-backed on purpose for the first tutorial implementation.
 * This keeps BLE protocol debugging independent from flash/NVS storage.
 */
#define FTR_MAX_FILE_SIZE       (16U * 1024U)

static uint8_t ftr_file[FTR_MAX_FILE_SIZE];
static size_t ftr_file_size;


/*
 * FILE_COUNT means "how many files are currently stored".
 * With the current single RAM slot implementation, this is therefore 0 or 1.
 */
static uint32_t ftr_file_count;

/*
 * Cumulative number of payload bytes from successfully completed uploads
 * since boot. This counter is intentionally session-scoped for now.
 */
static uint64_t ftr_total_uploaded_bytes;

static const struct bt_gatt_attr *file_count_attr;

static size_t ftr_upload_received;
static uint32_t ftr_upload_expected_size;
static uint8_t ftr_upload_expected_seq;
static uint8_t ftr_upload_batch_count;
static bool ftr_upload_batch_error;
static uint8_t ftr_upload_missing_seq;

static size_t ftr_download_offset;
static size_t ftr_download_batch_start_offset;
static uint8_t ftr_download_seq;
static uint8_t ftr_download_batch_start_seq;
static uint8_t ftr_download_chunks_in_batch;

static const struct bt_gatt_attr *ftr_tx_attr;

static void ftr_download_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(ftr_download_work, ftr_download_work_handler);

static uint32_t ftr_read_u32_le(const uint8_t *p)
{
	return ((uint32_t)p[0]) |
	       ((uint32_t)p[1] << 8) |
	       ((uint32_t)p[2] << 16) |
	       ((uint32_t)p[3] << 24);
}

static int ftr_tx_notify(const uint8_t *data, size_t len)
{
	if (!ftr_tx_notify_enabled || ftr_tx_attr == NULL) {
		printk("FTRF TX unavailable: client is not subscribed\n");
		return -EACCES;
	}

	return bt_gatt_notify(NULL, ftr_tx_attr, data, len);
}

static void ftr_send_control(uint8_t code, uint8_t seq)
{
	uint8_t msg[2] = { code, seq };
	int err = ftr_tx_notify(msg, sizeof(msg));

	if (err == 0) {
		printk("FTRF TX control: %02X %02X\n", code, seq);
	} else {
		printk("FTRF TX control failed: %d\n", err);
	}
}

static void ftr_abort(void)
{
	k_work_cancel_delayable(&ftr_download_work);

	ftr_state = FTR_STATE_IDLE;

	ftr_upload_received = 0U;
	ftr_upload_expected_size = 0U;
	ftr_upload_expected_seq = 0U;
	ftr_upload_batch_count = 0U;
	ftr_upload_batch_error = false;

	ftr_download_offset = 0U;
	ftr_download_batch_start_offset = 0U;
	ftr_download_seq = 0U;
	ftr_download_batch_start_seq = 0U;
	ftr_download_chunks_in_batch = 0U;

	printk("FTRF transfer aborted/reset\n");
}

static void ftr_start_upload(const uint8_t *buf, uint16_t len)
{
	ftr_abort();

	ftr_state = FTR_STATE_UPLOAD;
	ftr_upload_received = 0U;
	ftr_upload_expected_seq = 0U;
	ftr_upload_batch_count = 0U;
	ftr_upload_batch_error = false;
	ftr_upload_expected_size = 0U;

	if (len >= 5U) {
		ftr_upload_expected_size = ftr_read_u32_le(&buf[1]);
	}

	printk(
		"FTRF upload started. Expected size: %u byte(s), max: %u\n",
		(unsigned int)ftr_upload_expected_size,
		(unsigned int)FTR_MAX_FILE_SIZE
	);
}

static void ftr_process_upload_chunk(const uint8_t *buf, uint16_t len)
{
	const uint8_t marker = buf[0];
	const uint8_t seq = buf[1];
	const size_t payload_len = len - FTR_HEADER_SIZE;

	if (marker != FTR_ASCII_STR && marker != FTR_ASCII_ETX) {
		return;
	}

	if (seq != ftr_upload_expected_seq) {
		if (!ftr_upload_batch_error) {
			ftr_upload_missing_seq = ftr_upload_expected_seq;
		}
		ftr_upload_batch_error = true;

		printk(
			"FTRF upload sequence mismatch. Expected %02X, got %02X\n",
			ftr_upload_expected_seq,
			seq
		);
	} else {
		if (ftr_upload_received + payload_len > FTR_MAX_FILE_SIZE) {
			printk("FTRF upload exceeds RAM file buffer\n");
			ftr_send_control(FTR_ASCII_CAN, seq);
			ftr_abort();
			return;
		}

		memcpy(
			&ftr_file[ftr_upload_received],
			&buf[FTR_HEADER_SIZE],
			payload_len
		);

		ftr_upload_received += payload_len;
		ftr_upload_expected_seq++;
	}

	ftr_upload_batch_count++;

	const bool batch_boundary =
		(ftr_upload_batch_count >= FTR_CHUNKS_PER_ACK) ||
		(marker == FTR_ASCII_ETX);

	if (!batch_boundary) {
		return;
	}

	if (ftr_upload_batch_error) {
		ftr_send_control(FTR_ASCII_NACK, ftr_upload_missing_seq);

		/*
		 * Tutorial simplification:
		 * restart the current upload from the beginning after NACK.
		 * This still exercises the KMP ACK/NACK path while keeping the
		 * peripheral state machine intentionally small.
		 */
		printk("FTRF upload NACK: restarting upload stream\n");
		ftr_upload_received = 0U;
		ftr_upload_expected_seq = 0U;
		ftr_upload_batch_count = 0U;
		ftr_upload_batch_error = false;
		return;
	}

	ftr_send_control(FTR_ASCII_ACK, seq);
	ftr_upload_batch_count = 0U;

	if (marker == FTR_ASCII_ETX) {
		const bool file_count_changed = (ftr_file_count == 0U);

		ftr_file_size = ftr_upload_received;
		ftr_file_count = 1U;
		ftr_total_uploaded_bytes += ftr_file_size;

		printk(
			"FTRF upload complete: %u byte(s)\n",
			(unsigned int)ftr_file_size
		);

		printk(
			"FTRF stored files: %u, total uploaded bytes: %llu\n",
			(unsigned int)ftr_file_count,
			(unsigned long long)ftr_total_uploaded_bytes
		);

		if (ftr_upload_expected_size != 0U &&
		    ftr_upload_expected_size != ftr_file_size) {
			printk(
				"FTRF warning: header expected %u, received %u\n",
				(unsigned int)ftr_upload_expected_size,
				(unsigned int)ftr_file_size
			);
		}

		/*
		 * FILE_COUNT represents current storage occupancy. Since the current
		 * implementation has a single RAM slot, replacing the existing file
		 * keeps the count at 1 and therefore does not generate a notification.
		 */
		if (file_count_changed) {
			notify_file_count();
		}

		ftr_state = FTR_STATE_IDLE;
	}
}

static void ftr_start_download(void)
{
	if (!ftr_tx_notify_enabled) {
		printk("FTRF download rejected: subscribe to ...000C first\n");
		return;
	}

	k_work_cancel_delayable(&ftr_download_work);

	ftr_download_offset = 0U;
	ftr_download_batch_start_offset = 0U;
	ftr_download_seq = 0U;
	ftr_download_batch_start_seq = 0U;
	ftr_download_chunks_in_batch = 0U;
	ftr_state = FTR_STATE_DOWNLOAD_SENDING;

	printk(
		"FTRF download started: %u byte(s)\n",
		(unsigned int)ftr_file_size
	);

	k_work_reschedule(&ftr_download_work, K_NO_WAIT);
}

static void ftr_download_work_handler(struct k_work *work)
{
	uint8_t packet[FTR_CHUNK_SIZE];
	size_t remaining;
	size_t payload_len;
	bool final_chunk;
	int err;

	if (ftr_state != FTR_STATE_DOWNLOAD_SENDING) {
		return;
	}

	remaining = ftr_file_size - ftr_download_offset;
	payload_len = MIN(remaining, (size_t)FTR_PAYLOAD_SIZE);

	/*
	 * Empty file is represented by one ETX packet with zero payload.
	 * Otherwise ETX is used for the final payload packet.
	 */
	final_chunk =
		(ftr_file_size == 0U) ||
		(ftr_download_offset + payload_len >= ftr_file_size);

	packet[0] = final_chunk ? FTR_ASCII_ETX : FTR_ASCII_STR;
	packet[1] = ftr_download_seq;

	if (payload_len > 0U) {
		memcpy(
			&packet[FTR_HEADER_SIZE],
			&ftr_file[ftr_download_offset],
			payload_len
		);
	}

	err = ftr_tx_notify(
		packet,
		FTR_HEADER_SIZE + payload_len
	);

	if (err != 0) {
		printk("FTRF download notify failed: %d\n", err);

		/*
		 * Notifications may temporarily run out of controller buffers.
		 * Retry this same chunk shortly.
		 */
		k_work_reschedule(&ftr_download_work, K_MSEC(20));
		return;
	}

	printk(
		"FTRF TX chunk marker=%02X seq=%02X payload=%u\n",
		packet[0],
		packet[1],
		(unsigned int)payload_len
	);

	ftr_download_offset += payload_len;
	ftr_download_seq++;
	ftr_download_chunks_in_batch++;

	if (final_chunk ||
	    ftr_download_chunks_in_batch >= FTR_CHUNKS_PER_ACK) {
		ftr_state = FTR_STATE_DOWNLOAD_WAIT_ACK;
		printk("FTRF waiting for ACK/NACK\n");
		return;
	}

	k_work_reschedule(&ftr_download_work, K_MSEC(8));
}

static void ftr_process_download_control(
	const uint8_t *buf,
	uint16_t len)
{
	if (len < 1U) {
		return;
	}

	if (buf[0] == FTR_ASCII_CAN) {
		ftr_abort();
		return;
	}

	if (ftr_state != FTR_STATE_DOWNLOAD_WAIT_ACK) {
		return;
	}

	if (buf[0] == FTR_ASCII_ACK) {
		/*
		 * If the whole file has already been sent, this ACK completes
		 * the transfer. Otherwise start the next batch.
		 */
		if (ftr_download_offset >= ftr_file_size) {
			printk("FTRF download complete\n");
			ftr_state = FTR_STATE_IDLE;
			return;
		}

		ftr_download_batch_start_offset = ftr_download_offset;
		ftr_download_batch_start_seq = ftr_download_seq;
		ftr_download_chunks_in_batch = 0U;
		ftr_state = FTR_STATE_DOWNLOAD_SENDING;

		k_work_reschedule(&ftr_download_work, K_NO_WAIT);
		return;
	}

	if (buf[0] == FTR_ASCII_NACK) {
		printk("FTRF download NACK: resending current batch\n");

		ftr_download_offset = ftr_download_batch_start_offset;
		ftr_download_seq = ftr_download_batch_start_seq;
		ftr_download_chunks_in_batch = 0U;
		ftr_state = FTR_STATE_DOWNLOAD_SENDING;

		k_work_reschedule(&ftr_download_work, K_NO_WAIT);
	}
}

static ssize_t write_file_transfer_rx(
	struct bt_conn *conn,
	const struct bt_gatt_attr *attr,
	const void *buf,
	uint16_t len,
	uint16_t offset,
	uint8_t flags)
{
	const uint8_t *data = buf;

	if (!(flags & BT_GATT_WRITE_FLAG_CMD)) {
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
	}

	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len == 0U) {
		return len;
	}

	/*
	 * A transfer request is recognized only while idle. During an active
	 * transfer the same RX characteristic carries chunks and ACK/NACK/CAN.
	 */
	if (ftr_state == FTR_STATE_IDLE) {
		if (data[0] == FTR_DIRECTION_UPLOAD) {
			ftr_start_upload(data, len);
			return len;
		}

		if (data[0] == FTR_DIRECTION_DOWNLOAD) {
			ftr_start_download();
			return len;
		}

		if (data[0] == FTR_ASCII_CAN) {
			ftr_abort();
			return len;
		}
	}

	if (ftr_state == FTR_STATE_UPLOAD) {
		if (len >= FTR_HEADER_SIZE &&
		    (data[0] == FTR_ASCII_STR ||
		     data[0] == FTR_ASCII_ETX)) {
			ftr_process_upload_chunk(data, len);
		} else if (data[0] == FTR_ASCII_CAN) {
			ftr_abort();
		}

		return len;
	}

	if (ftr_state == FTR_STATE_DOWNLOAD_WAIT_ACK) {
		ftr_process_download_control(data, len);
		return len;
	}

	return len;
}


static ssize_t read_file_count(
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
		&ftr_file_count,
		sizeof(ftr_file_count)
	);
}

static ssize_t read_total_uploaded_bytes(
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
		&ftr_total_uploaded_bytes,
		sizeof(ftr_total_uploaded_bytes)
	);
}

static void file_count_ccc_changed(
	const struct bt_gatt_attr *attr,
	uint16_t value)
{
	file_count_notify_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk(
		"File count notifications %s\n",
		file_count_notify_enabled ? "enabled" : "disabled"
	);
}

static void notify_file_count(void)
{
	int err;

	if (!file_count_notify_enabled || file_count_attr == NULL) {
		return;
	}

	err = bt_gatt_notify(
		NULL,
		file_count_attr,
		&ftr_file_count,
		sizeof(ftr_file_count)
	);

	if (err == 0) {
		printk(
			"File count notification sent: %u\n",
			(unsigned int)ftr_file_count
		);
	} else {
		printk("File count notification failed: %d\n", err);
	}
}

static void file_transfer_tx_ccc_changed(
	const struct bt_gatt_attr *attr,
	uint16_t value)
{
	ftr_tx_notify_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk(
		"FTRF TX notifications %s\n",
		ftr_tx_notify_enabled ? "enabled" : "disabled"
	);

	if (!ftr_tx_notify_enabled &&
	    (ftr_state == FTR_STATE_DOWNLOAD_SENDING ||
	     ftr_state == FTR_STATE_DOWNLOAD_WAIT_ACK)) {
		ftr_abort();
	}
}

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
	),

	/* ...000B: FILE TRANSFER RX - WRITE WITHOUT RESPONSE */
	BT_GATT_CHARACTERISTIC(
		&tutorial_file_transfer_rx_uuid.uuid,
		BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		BT_GATT_PERM_WRITE_ENCRYPT,
		NULL,
		write_file_transfer_rx,
		NULL
	),

	/* ...000C: FILE TRANSFER TX - NOTIFY */
	BT_GATT_CHARACTERISTIC(
		&tutorial_file_transfer_tx_uuid.uuid,
		BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_NONE,
		NULL,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		file_transfer_tx_ccc_changed,
		BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT
	),

	/* ...000D: FILE COUNT - READ + NOTIFY, encrypted */
	BT_GATT_CHARACTERISTIC(
		&tutorial_file_count_uuid.uuid,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ_ENCRYPT,
		read_file_count,
		NULL,
		NULL
	),
	BT_GATT_CCC(
		file_count_ccc_changed,
		BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT
	),

	/* ...000E: TOTAL UPLOADED BYTES - READ, encrypted */
	BT_GATT_CHARACTERISTIC(
		&tutorial_total_uploaded_bytes_uuid.uuid,
		BT_GATT_CHRC_READ,
		BT_GATT_PERM_READ_ENCRYPT,
		read_total_uploaded_bytes,
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
	 * 22  Characteristic declaration ...000B
	 * 23  Characteristic value       ...000B
	 * 24  Characteristic declaration ...000C
	 * 25  Characteristic value       ...000C
	 * 26  CCCD                       ...000C
	 * 27  Characteristic declaration ...000D
	 * 28  Characteristic value       ...000D
	 * 29  CCCD                       ...000D
	 * 30  Characteristic declaration ...000E
	 * 31  Characteristic value       ...000E
	 */
	observable_value_attr = &tutorial_svc.attrs[8];
	periodic_notify_attr = &tutorial_svc.attrs[11];
	secure_state_attr = &tutorial_svc.attrs[20];
	ftr_tx_attr = &tutorial_svc.attrs[25];
	file_count_attr = &tutorial_svc.attrs[28];

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
