from abc import ABC, abstractmethod
from dataclasses import dataclass
from config.config import Config
from proto.request import MessageTypes
from exceptions.exceptions import InvalidPayloadResponseError, InvalidUUID
from enum import Enum
import struct


class ResPayload(ABC):
    @abstractmethod
    def size(self):
        pass

    @abstractmethod
    def to_bytes(self):
        pass


class RegistrationOkPayload(ResPayload):
    _RES_FMT = "!16s"
    _VALID_UUID_SZ = 16

    def __init__(self, uuid):
        super().__init__()

        if len(uuid) != RegistrationOkPayload._VALID_UUID_SZ:
            raise InvalidUUID()
        self._uuid = uuid

    def size(self):
        return struct.calcsize(RegistrationOkPayload._RES_FMT)

    def to_bytes(self):
        uuid = self._uuid.ljust(16, b"\x00")
        return struct.pack(RegistrationOkPayload._RES_FMT, uuid)


class ListUsersPayload(ResPayload):
    _RES_FMT = "!16s255s"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, users_list):
        super().__init__()
        self._users_list = users_list

    def size(self):
        return len(self._users_list) * ListUsersPayload._FMT_SZ

    def to_bytes(self):
        return b"".join(
            [
                struct.pack(
                    ListUsersPayload._RES_FMT,
                    user.uuid,
                    user.username.encode("utf-8").ljust(255, b"\x00"),
                )
                for user in self._users_list
            ]
        )


class PublicKeyPayload(ResPayload):
    _RES_FMT = "!16s160s"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, client_id, public_key):
        super().__init__()
        self._client_id = client_id
        self._public_key = public_key

    def size(self):
        return PublicKeyPayload._FMT_SZ

    def to_bytes(self):
        return struct.pack(
            PublicKeyPayload._RES_FMT,
            self._client_id,
            self._public_key.encode("utf-8").ljust(160, b"\x00"),
        )


class MessageSentPayload(ResPayload):
    _RES_FMT = "!16sI"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, dst_client_id, msg_id):
        super().__init__()
        self._dst_client_id = dst_client_id
        self._msg_id = msg_id

    def size(self):
        return MessageSentPayload._FMT_SZ

    def to_bytes(self):
        return struct.pack(
            MessageSentPayload._RES_FMT, self._dst_client_id, self._msg_id
        )


class PollMessagePayload(ResPayload):
    _RES_FMT = "!16sIBI"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, client_id, msg_id, msg_type: MessageTypes, msg_sz, content):
        super().__init__()
        self._client_id = client_id
        self._msg_id = msg_id
        self._msg_type = msg_type
        self._msg_sz = msg_sz
        self._content = content

    def size(self):
        return PollMessagePayload._FMT_SZ + len(
            self._content.encode("utf-8").ljust(self._msg_sz, b"\x00")
        )

    def to_bytes(self):
        return struct.pack(
            PollMessagePayload._RES_FMT,
            self._client_id,
            self._msg_id,
            self._msg_type.value,
            self._msg_sz,
        ) + self._content.encode("utf-8").ljust(self._msg_sz, b"\x00")


class ErrorResponse(ResPayload):
    pass


class ResponseCodes(Enum):
    REG_OK = 2100
    LIST_USRS = 2101
    PUB_KEY = 2102
    MSG_SENT = 2103
    POLL_MSGS = 2104
    ERROR = 9000

    @staticmethod
    def code_to_enum(code):
        if code == 2100:
            return ResponseCodes.REG_OK
        elif code == 2101:
            return ResponseCodes.LIST_USRS
        elif code == 2102:
            return ResponseCodes.PUB_KEY
        elif code == 2103:
            return ResponseCodes.MSG_SENT
        elif code == 2104:
            return ResponseCodes.POLL_MSGS
        return ResponseCodes.ERROR


class Response:
    @dataclass
    class Header:
        _HEADER_FMT = "!BHI"

        version: int
        code: int
        payload_sz: int

        def to_bytes(self):
            return struct.pack(
                Response.Header._HEADER_FMT,
                self.version,
                self.code.value,
                self.payload_sz,
            )

    def __init__(self, code, payload):
        if not isinstance(payload, ResPayload) or not isinstance(code, ResponseCodes):
            raise InvalidPayloadResponseError()

        self._header = Response.Header(Config.VERSION, code, payload.size())
        self._payload = payload

    def to_bytes(self):
        return self._header.to_bytes() + self._payload.to_bytes()


class ResponseFactory:
    _builders = {
        ResponseCodes.REG_OK: lambda uuid: RegistrationOkPayload(uuid),
        ResponseCodes.LIST_USRS: lambda users_list: ListUsersPayload(users_list),
        ResponseCodes.PUB_KEY: lambda client_id, public_key: PublicKeyPayload(
            client_id, public_key
        ),
        ResponseCodes.MSG_SENT: lambda dst_client_id, msg_id: MessageSentPayload(
            dst_client_id, msg_id
        ),
        ResponseCodes.POLL_MSGS: lambda client_id,
        msg_id,
        msg_type,
        msg_sz,
        content: PollMessagePayload(client_id, msg_id, msg_type, msg_sz, content),
        ResponseCodes.ERROR: lambda: ErrorResponse(),
    }

    @staticmethod
    def create_response(code: ResponseCodes, *args, **kwargs) -> Response:
        builder = ResponseFactory._builders.get(code)
        if builder is None:
            raise InvalidPayloadResponseError(
                f"No builder found for response code {code}"
            )
        payload = builder(*args, **kwargs)
        return Response(code, payload)
