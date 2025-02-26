# mini_test.py
from config.config import Config
from proto.request import Request
from proto.response import (
    RegistrationOkPayload,
    ListUsersPayload,
    ResponseCodes,
    Response,
)
import struct


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


def test_code_603():
    client_id = b"1234567890123456"
    version = 1
    code = 603

    message_type = 1
    message_content = b"Hello, this is a test message."
    content_size = len(message_content)

    payload_fixed = struct.pack("<16sBI", client_id, message_type, content_size)
    payload = payload_fixed + message_content

    payload_size = len(payload)

    packet = (
        struct.pack(Request._HEADER_FMT, client_id, version, code, payload_size)
        + payload
    )

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())
    print()


def test_code_604():
    client_id = b"1234567890123456"
    version = 1
    code = 604

    packet = struct.pack(Request._HEADER_FMT, client_id, version, code, 0)

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())
    print()


def test_response_2100():
    import uuid

    uuid = uuid.uuid4()

    res = Response(ResponseCodes.REG_OK, RegistrationOkPayload(uuid.bytes))
    print(res)
    print(res.to_bytes())
    print()


def test_response_2101():
    import uuid
    from dataclasses import dataclass

    uuid = uuid.uuid4()

    @dataclass
    class User:
        uuid: bytes
        username: str

    users = []
    for i in range(5):
        users.append(User(uuid.bytes, f"user-{i}"))

    res = Response(ResponseCodes.USRS_LIST_OK, ListUsersPayload(users))
    print(res)
    print(res.to_bytes())
    print()


def main():
    Config.load()
    print(f"port: {Config.PORT}")

    try:
        test_code_600()
        test_code_601()
        test_code_602()
        test_code_603()
        test_code_604()
        test_response_2100()
        test_response_2101()
    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
