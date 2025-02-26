from config.config import Config
from proto.request import Request, MessageTypes
from proto.response import (
    RegistrationOkPayload,
    ListUsersPayload,
    PublicKeyPayload,
    MessageSentPayload,
    PollMessagePayload,
    ResponseCodes,
    Response,
)
import struct
import uuid
from dataclasses import dataclass


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
    new_uuid = uuid.uuid4()
    res = Response(ResponseCodes.REG_OK, RegistrationOkPayload(new_uuid.bytes))
    print(res)
    print(res.to_bytes())
    print()


def test_response_2101():
    new_uuid = uuid.uuid4()

    @dataclass
    class User:
        uuid: bytes
        username: str

    users = []
    for i in range(5):
        users.append(User(new_uuid.bytes, f"user-{i}"))
    res = Response(ResponseCodes.LIST_USRS, ListUsersPayload(users))
    print(res)
    print(res.to_bytes())
    print()


def test_response_2102():
    new_uuid = uuid.uuid4()
    client_id = new_uuid.bytes
    public_key = "public_key_value"
    res = Response(ResponseCodes.PUB_KEY, PublicKeyPayload(client_id, public_key))
    print(res)
    print(res.to_bytes())
    print()


def test_response_2103():
    new_uuid = uuid.uuid4()
    dst_client_id = new_uuid.bytes
    msg_id = 1234
    res = Response(ResponseCodes.MSG_SENT, MessageSentPayload(dst_client_id, msg_id))
    print(res)
    print(res.to_bytes())
    print()


def test_response_2104():
    new_uuid = uuid.uuid4()
    client_id = new_uuid.bytes
    msg_id = 5678
    msg_type = MessageTypes.SEND_TXT
    msg_content = "Test poll message"
    msg_sz = len(msg_content)
    res = Response(
        ResponseCodes.POLL_MSGS,
        PollMessagePayload(client_id, msg_id, msg_type, msg_sz, msg_content),
    )
    print(res)
    print(res.to_bytes())
    print()


def main():
    Config.load()
    print(f"port: {Config.PORT}")
    try:
        # test_code_600()
        # test_code_601()
        # test_code_602()
        # test_code_603()
        # test_code_604()
        # test_response_2100()
        # test_response_2101()
        # test_response_2102()
        test_response_2103()
        # test_response_2104()
    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
