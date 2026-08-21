
# BLE-MIRABILIS-BLUE File Transfer Test

## Install

```bash
python3 -m venv .venv
source .venv/bin/activate

pip install bleak
```

## Prepare a test file

```bash
head -c 4096 /dev/urandom > demo_upload.bin
```

or

```bash
echo "Hello BLE Mirabilis!" > demo_upload.bin
```

## Run

```bash
python3 ble_mirabilis_file_transfer_test.py
```

## What you'll see

```
Scanning for BLE-MIRABILIS-BLUE...
Connected.

Uploading 4096 bytes...

Batch 0
TX seq=00 size=18
TX seq=01 size=18
...
RX: 06 07
Batch ACK (07)

...

Starting download...

RX: 02 00 ...
RX: 02 01 ...
...
RX: 03 91 ...

ACK 91

Downloaded 4096 bytes.
```

The downloaded file is written to `downloaded.bin`.

You can compare both files:

```bash
cmp demo_upload.bin downloaded.bin
```

If `cmp` prints nothing, the files are identical.
