from abc import ABC, abstractmethod
from dataclasses import dataclass
from config.config import Config
from entities.message_entity import MessageEntity
from exceptions.exceptions import InvalidPayloadResponseError, InvalidUUID
from enum import Enum
import struct


class ResPayload(ABC):
    """Abstract class for response payloads"""

    @abstractmethod
    def size(self):
        """Gets the size of the payload"""
        pass

    @abstractmethod
    def to_bytes(self):
        """Converts the payload to bytes"""
        pass


class RegistrationOkPayload(ResPayload):
    """Response payload for registration success"""

    _RES_FMT = "<16s"
    _VALID_UUID_SZ = 16

    def __init__(self, uuid):
        super().__init__()

        if len(uuid) != RegistrationOkPayload._VALID_UUID_SZ:
            raise InvalidUUID("Invalid UUID length")
        self._uuid = uuid

    def size(self):
        return struct.calcsize(RegistrationOkPayload._RES_FMT)

    def to_bytes(self):
        uuid = self._uuid.ljust(16, b"\x00")
        return struct.pack(RegistrationOkPayload._RES_FMT, uuid)


class ListUsersPayload(ResPayload):
    """Response payload for listing users"""

    _RES_FMT = "<16s255s"
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
                    user.get_uuid(),
                    user.get_username().ljust(255, b"\x00"),
                )
                for user in self._users_list
            ]
        )


class PublicKeyPayload(ResPayload):
    """Response payload for public key"""

    _RES_FMT = "<16s160s"
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
            self._public_key.ljust(160, b"\x00"),
        )


class MessageSentPayload(ResPayload):
    """Response payload for message sent from a client to another client"""

    _RES_FMT = "<16sI"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, dst_client_id, msg_id):
        super().__init__()
        self._dst_client_id = dst_client_id
        self._msg_id = msg_id

    def size(self):
        return MessageSentPayload._FMT_SZ

    def to_bytes(self):
        print(self._dst_client_id)
        print(self._msg_id)
        return struct.pack(
            MessageSentPayload._RES_FMT, self._dst_client_id, self._msg_id
        )


class PollMessagePayload(ResPayload):
    """Response payload for polling messages"""

    _RES_FMT = "<16sIBI"
    _FMT_SZ = struct.calcsize(_RES_FMT)

    def __init__(self, msgs: list[MessageEntity]):
        super().__init__()
        self._msgs = msgs

    def size(self):
        return len(self._msgs) * PollMessagePayload._FMT_SZ + sum(
            len(msg.get_content()) for msg in self._msgs
        )

    def to_bytes(self):
        to_send = b""
        for msg in self._msgs:
            to_send += (
                struct.pack(
                    PollMessagePayload._RES_FMT,
                    msg.get_from_client(),
                    msg.get_id(),
                    msg.get_msg_type().value,
                    len(msg.get_content()),
                )
                + msg.get_content()
            )

        return to_send


class ErrorResponse(ResPayload):
    """Response payload for error"""

    def __init__(self):
        super().__init__()

    def size(self):
        return 0

    def to_bytes(self):
        return b""


class ResponseCodes(Enum):
    """Response codes"""

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
    """Response class"""

    @dataclass
    class Header:
        """Header class for the response"""

        _HEADER_FMT = "<BHI"

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
            raise InvalidPayloadResponseError("Invalid payload or code")

        # Create the header
        self._header = Response.Header(Config.VERSION, code, payload.size())
        # Store the payload
        self._payload = payload

    def to_bytes(self):
        """Convert the header and payload to bytes"""
        return self._header.to_bytes() + self._payload.to_bytes()


class ResponseFactory:
    """Factory class for creating responses"""

    # Builders for the different response types
    _builders = {
        ResponseCodes.REG_OK: lambda uuid: RegistrationOkPayload(uuid),
        ResponseCodes.LIST_USRS: lambda users_list: ListUsersPayload(users_list),
        ResponseCodes.PUB_KEY: lambda client_id, public_key: PublicKeyPayload(
            client_id, public_key
        ),
        ResponseCodes.MSG_SENT: lambda dst_client_id, msg_id: MessageSentPayload(
            dst_client_id, msg_id
        ),
        ResponseCodes.POLL_MSGS: lambda msgs: PollMessagePayload(msgs),
        ResponseCodes.ERROR: lambda: ErrorResponse(),
    }

    @staticmethod
    def create_response(code: ResponseCodes, *args, **kwargs) -> Response:
        """Creates a response based on the code and the arguments"""

        # Gets the builder for the response code
        builder = ResponseFactory._builders.get(code)
        if builder is None:
            raise InvalidPayloadResponseError(
                f"No builder found for response code {code}"
            )
        # Creates the payload
        payload = builder(*args, **kwargs)
        return Response(code, payload)
