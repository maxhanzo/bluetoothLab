
#!/usr/bin/env python3
"""
BLE-MIRABILIS-BLUE File Metadata Monitor
========================================

Reads and monitors the secure file metadata characteristics:

000D FILE_COUNT
    READ | NOTIFY
    uint32 little-endian
    Current number of files stored on the device.

000E TOTAL_UPLOADED_BYTES
    READ
    uint64 little-endian
    Cumulative number of successfully uploaded bytes since boot.

Requirement:
    pip install bleak
"""

import asyncio
from bleak import BleakScanner, BleakClient

DEVICE_NAME = "BLE-MIRABILIS-BLUE"

FILE_COUNT_UUID = "7E57A000-0000-4B1A-9C00-00000000000D"
TOTAL_UPLOADED_BYTES_UUID = "7E57A000-0000-4B1A-9C00-00000000000E"


class MetadataMonitor:
    def __init__(self):
        self.client = None

    async def find_device(self):
        print(f"Scanning for {DEVICE_NAME}...")

        device = await BleakScanner.find_device_by_name(
            DEVICE_NAME,
            timeout=10.0
        )

        if device is None:
            raise RuntimeError(
                f"{DEVICE_NAME} not found. "
                "Make sure it is advertising and not connected to another central."
            )

        print(f"Found: {device.address}")
        return device

    async def connect(self):
        device = await self.find_device()

        self.client = BleakClient(device)

        print("Connecting...")
        await self.client.connect()

        print("Connected.")

    async def read_file_count(self):
        raw = await self.client.read_gatt_char(FILE_COUNT_UUID)

        if len(raw) != 4:
            raise RuntimeError(
                f"Unexpected FILE_COUNT size: {len(raw)} byte(s)"
            )

        value = int.from_bytes(raw, byteorder="little", signed=False)

        print(
            f"FILE_COUNT: {value} "
            f"(raw: {bytes(raw).hex(' ')})"
        )

        return value

    async def read_total_uploaded_bytes(self):
        raw = await self.client.read_gatt_char(
            TOTAL_UPLOADED_BYTES_UUID
        )

        if len(raw) != 8:
            raise RuntimeError(
                "Unexpected TOTAL_UPLOADED_BYTES size: "
                f"{len(raw)} byte(s)"
            )

        value = int.from_bytes(raw, byteorder="little", signed=False)

        print(
            f"TOTAL_UPLOADED_BYTES: {value} "
            f"(raw: {bytes(raw).hex(' ')})"
        )

        return value

    def file_count_notification_handler(self, characteristic, data):
        raw = bytes(data)

        if len(raw) != 4:
            print(
                "FILE_COUNT notification with unexpected "
                f"length: {len(raw)}"
            )
            return

        value = int.from_bytes(raw, byteorder="little", signed=False)

        print(
            "\n*** FILE_COUNT NOTIFICATION ***"
        )
        print(
            f"FILE_COUNT changed to: {value} "
            f"(raw: {raw.hex(' ')})"
        )
        print(
            "*******************************\n"
        )

    async def subscribe_file_count(self):
        print("Subscribing to FILE_COUNT notifications...")

        await self.client.start_notify(
            FILE_COUNT_UUID,
            self.file_count_notification_handler
        )

        print("Subscribed.")

    async def run(self):
        await self.connect()

        print("\nInitial secure metadata:")
        print("------------------------")

        await self.read_file_count()
        await self.read_total_uploaded_bytes()

        print()

        await self.subscribe_file_count()

        print()
        print("Monitoring FILE_COUNT.")
        print("Upload files from another client to observe notifications.")
        print("Press Ctrl+C to quit.")
        print()

        try:
            while self.client.is_connected:
                await asyncio.sleep(1)
        finally:
            if self.client and self.client.is_connected:
                try:
                    await self.client.stop_notify(FILE_COUNT_UUID)
                except Exception:
                    pass

                await self.client.disconnect()

                print("Disconnected.")


async def main():
    monitor = MetadataMonitor()
    await monitor.run()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nStopped by user.")
