# mini_test.py
from config.config import Config
import struct
from proto.request import Request


def test_code_600():
    client_id = b"1234567890123456"
    version = 1
    code = 600

    username = b"username"
    public_key = b"public_key"

    username_padded = username.ljust(255, b"\x00")
    public_key_padded = public_key.ljust(160, b"\x00")
    payload = username_padded + public_key_padded
    payload_size = len(payload)

    packet = (
        struct.pack(Request._HEADER_FMT, client_id, version, code, payload_size)
        + payload
    )

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())
    print()


def test_code_601():
    client_id = b"1234567890123456"
    version = 1
    code = 601

    packet = struct.pack(Request._HEADER_FMT, client_id, version, code, 0)

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())
    print()


def test_code_602():
    client_id = b"1234567890123456"
    version = 1
    code = 602
    payload_size = len(client_id)

    packet = (
        struct.pack(Request._HEADER_FMT, client_id, version, code, payload_size)
        + client_id
    )

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())
    print()


def main():
    Config.load()
    print(f"port: {Config.PORT}")

    test_code_600()
    test_code_601()
    test_code_602()


if __name__ == "__main__":
    main()
