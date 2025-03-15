from proto.context import Context
from proto.response import ResponseCodes, ResponseFactory, Response
from proto.request import (
    Request,
    RequestCodes,
    RegistrationPayload,
    GetPublicKeyPayload,
    SendMessagePayload,
)
from services.client_service import ClientService
from services.message_service import MessagesService
import logging
import binascii

logger = logging.getLogger(__name__)


def hexify(id: bytes) -> str:
    return binascii.hexlify(id).decode("utf-8")


class Controller:
    """Business layer for the server"""

    def __init__(
        self,
        client_service: ClientService,
        messages_service: MessagesService,
    ):
        self._client_service = client_service
        self._messages_service = messages_service
        self._hanlders = dict()
        self._install_handlers()

    def _install_handlers(self):
        # Setup the handlers for the different routes
        self._hanlders[RequestCodes.REGISTER.value] = self._register
        self._hanlders[RequestCodes.LIST_USERS.value] = self._list_users
        self._hanlders[RequestCodes.GET_PUB_KEY.value] = self._get_pub_key
        self._hanlders[RequestCodes.SEND_MSG.value] = self._send_msg
        self._hanlders[RequestCodes.POLL_MSGS.value] = self._poll_msgs

    def dispatch(self, conn, packet):
        """Receives a packet, parses the header and payload and dispatches the appropriate handler"""
        try:
            ctx = Context(conn, Request(packet))
            code = ctx.get_req().get_header().code
            payload = ctx.get_req().get_payload()
            self._hanlders[code](ctx, payload)
        except Exception as e:
            logger.exception(e)
            conn.send(ResponseFactory.create_response(ResponseCodes.ERROR).to_bytes())

    def _register(
        self, ctx: Context, register_payload: RegistrationPayload
    ) -> Response:
        """Router handler for registering a new client"""
        new_client = self._client_service.create(register_payload)
        logger.info("New client registered")
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.REG_OK,
                new_client.get_uuid(),
            )
        )

    def _list_users(self, ctx: Context, _) -> Response:
        """Router handler for fetching all registered users except for the user who made the request"""
        client_id = ctx.get_req().get_header().client_id
        users_list = self._client_service.find_all(client_id)
        users_list = list(filter(lambda x: x.get_uuid() != client_id, users_list))
        logger.info(f"Sending users list to {hexify(client_id)}")
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.LIST_USRS,
                users_list,
            )
        )

    def _get_pub_key(
        self, ctx: Context, get_pub_key_payload: GetPublicKeyPayload
    ) -> Response:
        """Handler for fetching the public key of a user"""
        user = self._client_service.find_by_id(get_pub_key_payload.user_id)
        logger.info(
            f"Sending public key to {hexify(ctx.get_req().get_header().client_id)}"
        )
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.PUB_KEY,
                user.get_uuid(),
                user.get_public_key(),
            )
        )

    def _send_msg(self, ctx: Context, send_msg_payload: SendMessagePayload) -> Response:
        """Handler for sending a message"""
        client_id = ctx.get_req().get_header().client_id
        msg = self._messages_service.create(client_id, send_msg_payload)
        logger.info(
            f"Message sent from {hexify(client_id)} to {hexify(msg.get_to_client())}"
        )
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.MSG_SENT,
                msg.get_to_client(),
                msg.get_id(),
            )
        )

    def _poll_msgs(self, ctx: Context, _) -> Response:
        """Handler for polling pending messages"""
        client_id = ctx.get_req().get_header().client_id
        msgs = self._messages_service.poll_msgs(client_id)
        logger.info(f"Polling messages({len(msgs)}) for {hexify(client_id)}")
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.POLL_MSGS,
                msgs,
            )
        )
