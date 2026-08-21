
#!/usr/bin/env python3
"""
BLE-MIRABILIS-BLUE File Transfer Test
=====================================

Tests the tutorial file-transfer protocol (v0.6.1).

Requirements:
    pip install bleak

Author: ChatGPT + Max Ueda (tutorial project)
"""

import asyncio
import struct
from pathlib import Path
from bleak import BleakScanner, BleakClient

DEVICE_NAME = "BLE-MIRABILIS-BLUE"

FILE_TRANSFER_RX = "7E57A000-0000-4B1A-9C00-00000000000B"
FILE_TRANSFER_TX = "7E57A000-0000-4B1A-9C00-00000000000C"

STR  = 0x02
ETX  = 0x03
ACK  = 0x06
NACK = 0x15
CAN  = 0x18

UPLOAD_CMD   = ord("U")
DOWNLOAD_CMD = ord("D")

CHUNK_SIZE   = 20
HEADER_SIZE  = 2
PAYLOAD_SIZE = CHUNK_SIZE - HEADER_SIZE
CHUNKS_PER_ACK = 8


class MirabilisTester:

    def __init__(self):
        self.client = None
        self.rx_queue = asyncio.Queue()

    async def connect(self):
        print(f"Scanning for {DEVICE_NAME}...")

        device = None
        for d in await BleakScanner.discover(timeout=5):
            if d.name == DEVICE_NAME:
                device = d
                break

        if device is None:
            raise RuntimeError("Device not found.")

        print(f"Found: {device.address}")

        self.client = BleakClient(device)

        await self.client.connect()

        print("Connected.")

        await self.client.start_notify(
            FILE_TRANSFER_TX,
            self.notification_handler
        )

        print("Subscribed to TX notifications.")

    def notification_handler(self, characteristic, data):
        print("RX:", data.hex(" "))
        self.rx_queue.put_nowait(bytes(data))

    async def upload(self, file_path: str):

        data = Path(file_path).read_bytes()

        print(f"\nUploading {len(data)} bytes...")

        header = bytes([UPLOAD_CMD]) + struct.pack("<I", len(data))

        await self.client.write_gatt_char(
            FILE_TRANSFER_RX,
            header,
            response=False
        )

        seq = 0

        for batch in range(0, (len(data)+PAYLOAD_SIZE-1)//PAYLOAD_SIZE, CHUNKS_PER_ACK):

            print(f"\nBatch {batch//CHUNKS_PER_ACK}")

            for i in range(CHUNKS_PER_ACK):

                index = batch + i

                offset = index * PAYLOAD_SIZE

                if offset >= len(data):
                    break

                payload = data[offset:offset+PAYLOAD_SIZE]

                marker = ETX if offset + len(payload) >= len(data) else STR

                packet = bytes([marker, seq]) + payload

                print(f"  TX seq={seq:02X}  size={len(payload)}")

                await self.client.write_gatt_char(
                    FILE_TRANSFER_RX,
                    packet,
                    response=False
                )

                seq = (seq + 1) & 0xFF

                await asyncio.sleep(0.005)

            ack = await self.rx_queue.get()

            if ack[0] == ACK:
                print(f"Batch ACK ({ack[1]:02X})")

            elif ack[0] == NACK:
                print(f"Batch NACK ({ack[1]:02X})")
                raise RuntimeError("Upload failed.")

        print("\nUpload complete.")

    async def download(self, output_file):

        print("\nStarting download...")

        await self.client.write_gatt_char(
            FILE_TRANSFER_RX,
            bytes([DOWNLOAD_CMD]),
            response=False
        )

        output = bytearray()

        batch = []

        while True:

            packet = await self.rx_queue.get()

            marker = packet[0]
            seq = packet[1]

            payload = packet[2:]

            print(
                f"Chunk seq={seq:02X} "
                f"marker={marker:02X} "
                f"payload={len(payload)}"
            )

            output.extend(payload)

            batch.append(seq)

            need_ack = (
                len(batch) == CHUNKS_PER_ACK
                or marker == ETX
            )

            if need_ack:

                await self.client.write_gatt_char(
                    FILE_TRANSFER_RX,
                    bytes([ACK, batch[-1]]),
                    response=False
                )

                print(f"ACK {batch[-1]:02X}")

                batch.clear()

            if marker == ETX:
                break

        Path(output_file).write_bytes(output)

        print(f"\nDownloaded {len(output)} bytes.")
        print(f"Saved to: {output_file}")

    async def disconnect(self):

        await self.client.disconnect()

        print("Disconnected.")


async def main():

    tester = MirabilisTester()

    await tester.connect()

    await tester.upload("demo_upload.bin")

    await tester.download("downloaded.bin")

    await tester.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
