import struct
from dataclasses import dataclass
from enum import Enum
from abc import ABC, abstractmethod

from exceptions.exceptions import InvalidCodeError, InvalidPayloadError


class Payload(ABC):
    @classmethod
    @abstractmethod
    def from_bytes(cls, data, data_len=0):
        pass


@dataclass
class RegistrationPayload(Payload):
    _PAYLOAD_FMT = "<255B160B"

    username: str
    public_key: str

    @classmethod
    def from_bytes(cls, data, data_len=0):
        try:
            data = struct.unpack(RegistrationPayload._PAYLOAD_FMT, data)
            username = bytes(data[:255]).rstrip(b"\x00").decode("utf-8")
            key = bytes(data[255:]).rstrip(b"\x00").decode("utf-8")

            return cls(username, key)
        except Exception:
            raise InvalidPayloadError()


@dataclass
class ListUsersPayload(Payload):
    @classmethod
    def from_bytes(cls, data, data_len=0):
        if len(data) or data_len != 0:
            raise InvalidPayloadError()


@dataclass
class GetPublicKeyPayload(Payload):
    _PAYLOAD_FMT = "<16s"

    user_id: str

    @classmethod
    def from_bytes(cls, data, data_len=0):
        try:
            data = struct.unpack(GetPublicKeyPayload._PAYLOAD_FMT, data)
            (client_id,) = data
            return cls(client_id.decode("utf-8"))
        except Exception:
            raise InvalidPayloadError()


class RequestCodes(Enum):
    REGISTER = 600
    LIST_USERS = 601
    GET_PUB_KEY = 602

    def code_to_enum(code):
        if code == 600:
            return RequestCodes.REGISTER
        elif code == 601:
            return RequestCodes.LIST_USERS
        elif code == 602:
            return RequestCodes.GET_PUB_KEY


class Request:
    _HEADER_FMT = "<16sBHI"
    _HEADER_SZ = struct.calcsize(_HEADER_FMT)
    _PAYLOAD_CLASSES = {}

    @dataclass
    class Header:
        client_id: str
        version: int
        code: int

    def __init__(self, packet):
        data = struct.unpack(Request._HEADER_FMT, packet[: Request._HEADER_SZ])
        client_id, version, code, payload_size = data
        self._header = Request.Header(client_id.decode("utf-8"), version, code)

        code = RequestCodes.code_to_enum(code)
        payload_cls = Request._PAYLOAD_CLASSES.get(code)
        if payload_cls is None:
            raise InvalidCodeError(f"Error: {code} is invalid")

        raw_payload = packet[Request._HEADER_SZ : Request._HEADER_SZ + payload_size]
        self._payload = payload_cls.from_bytes(raw_payload, payload_size)

    def get_header(self):
        return self._header

    def get_payload(self):
        return self._payload


Request._PAYLOAD_CLASSES[RequestCodes.REGISTER] = RegistrationPayload
Request._PAYLOAD_CLASSES[RequestCodes.LIST_USERS] = ListUsersPayload
Request._PAYLOAD_CLASSES[RequestCodes.GET_PUB_KEY] = GetPublicKeyPayload
