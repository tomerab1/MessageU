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

logger = logging.getLogger(__name__)


class Controller:
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
        self._hanlders[RequestCodes.REGISTER.value] = self._register
        self._hanlders[RequestCodes.LIST_USERS.value] = self._list_users
        self._hanlders[RequestCodes.GET_PUB_KEY.value] = self._get_pub_key
        self._hanlders[RequestCodes.SEND_MSG.value] = self._send_msg
        self._hanlders[RequestCodes.POLL_MSGS.value] = self._poll_msgs

    def dispatch(self, conn, packet):
        try:
            ctx = Context(conn, Request(packet))
            code = ctx.get_req().get_header().code
            payload = ctx.get_req().get_payload()
            res = self._hanlders[code](ctx, payload)

            return res
        except Exception as e:
            logger.warning(e)
            conn.send(ResponseFactory.create_response(ResponseCodes.ERROR).to_bytes())

    def _register(
        self, ctx: Context, register_payload: RegistrationPayload
    ) -> Response:
        new_client = self._client_service.create(register_payload)
        print(new_client)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.REG_OK,
                new_client.get_uuid(),
            )
        )

    def _list_users(self, ctx: Context, _) -> Response:
        users_list = self._client_service.find_all()
        print(users_list)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.LIST_USRS,
                users_list,
            )
        )

    def _get_pub_key(
        self, ctx: Context, get_pub_key_payload: GetPublicKeyPayload
    ) -> Response:
        user = self._client_service.find_by_id(get_pub_key_payload.user_id)
        print(user)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.PUB_KEY,
                user.get_uuid(),
                user.get_public_key(),
            )
        )

    def _send_msg(self, ctx: Context, send_msg_payload: SendMessagePayload) -> Response:
        client_id = ctx.get_req().get_header().client_id
        msg = self._messages_service.create(client_id, send_msg_payload)
        print(msg)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.MSG_SENT,
                msg.get_to_client(),
                msg.get_uuid(),
            )
        )

    def _poll_msgs(self, ctx: Context, _) -> Response:
        client_id = ctx.get_req().get_header().client_id
        msgs = self._messages_service.poll_msgs(client_id)
        print(msgs)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.POLL_MSGS,
                msgs,
            )
        )
