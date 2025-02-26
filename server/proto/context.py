from proto.request import Request
from proto.response import Response


class Context:
    def __init__(self, socket, request: Request):
        self._socket = socket
        self._request = request

    def get_req(self) -> Request:
        return self._request

    def write(self, response: Response):
        self._socket.send(response)
