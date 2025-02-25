from config.config import Config
import struct
from proto.request import Request


def main():
    Config.load()

    print(f"port: {Config.PORT}")

    client_id = b"1234567890123456"
    version = 1
    code = 1000
    payload = b"Hello, world!"
    payload_size = len(payload)

    packet = (
        struct.pack(Request._HEADER_FMT, client_id, version, code, payload_size)
        + payload
    )

    req = Request(packet)
    print("Header:", req.get_header())
    print("Payload:", req.get_payload())


if __name__ == "__main__":
    main()
