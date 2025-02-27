from proto.context import Context
from proto.response import ResponseCodes, ResponseFactory, Response
from proto.request import RequestCodes, RegistrationPayload
from services.client_service import ClientService
from services.message_service import MessagesService


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

    def multiplex(self, ctx: Context):
        try:
            code = ctx.get_req().get_header().code
            payload = ctx.get_req().get_payload()
            res = self._hanlders[code](ctx, payload)
            return res
        except Exception as e:
            print(e)

    def register(self, ctx: Context, register_payload: RegistrationPayload) -> Response:
        new_client = self._client_service.create(register_payload)
        ctx.write(
            ResponseFactory.create_response(ResponseCodes.REG_OK, new_client.get_uuid())
        )
        print(new_client)
