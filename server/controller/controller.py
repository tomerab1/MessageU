from proto.context import Context
from proto.response import ResponseCodes, ResponseFactory, Response
from proto.request import (
    Request,
    RequestCodes,
    RegistrationPayload,
    GetPublicKeyPayload,
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
        self._hanlders[RequestCodes.REGISTER.value] = self.register
        self._hanlders[RequestCodes.LIST_USERS.value] = self.list_users
        self._hanlders[RequestCodes.GET_PUB_KEY.value] = self.get_pub_key

    def dispatch(self, conn, packet):
        try:
            ctx = Context(conn, Request(packet))

            code = ctx.get_req().get_header().code
            payload = ctx.get_req().get_payload()
            res = self._hanlders[code](ctx, payload)
            return res
        except Exception as e:
            print(e)
            print(ResponseFactory.create_response(ResponseCodes.ERROR))
            conn.send(ResponseFactory.create_response(ResponseCodes.ERROR).to_bytes())

    def register(self, ctx: Context, register_payload: RegistrationPayload) -> Response:
        new_client = self._client_service.create(register_payload)
        print(new_client)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.REG_OK,
                new_client.get_uuid(),
            )
        )

    def list_users(self, ctx: Context, _) -> Response:
        users_list = self._client_service.find_all()
        print(users_list)
        ctx.write(
            ResponseFactory.create_response(
                ResponseCodes.LIST_USRS,
                users_list,
            )
        )

    def get_pub_key(
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
