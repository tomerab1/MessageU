import struct
from dataclasses import dataclass


class Request:
    _HEADER_FMT = "<16sBHI"
    _HEADER_SZ = struct.calcsize(_HEADER_FMT)

    @dataclass
    class Header:
        client_id: str
        version: int
        code: int

    @dataclass
    class Payload:
        payload: bytes

    def __init__(self, packet):
        data = struct.unpack(Request._HEADER_FMT, packet[: Request._HEADER_SZ])
        client_id, version, code, payload_size = data

        self._header = Request.Header(client_id, version, code)
        self._payload = Request.Payload(
            packet[Request._HEADER_SZ : Request._HEADER_SZ + payload_size]
        )

    def get_header(self):
        return self._header

    def get_payload(self):
        return self._payload
