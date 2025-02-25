import struct


class Request:
    _HEADER_FMT = "!16sBHI"
    _HEADER_SZ = struct.calcsize(_HEADER_FMT)

    def __init__(self, packet):
        self._raw_data = struct.unpack(Request._HEADER_FMT, packet)
