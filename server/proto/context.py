from proto.request import Request
from proto.response import Response


class Context:
    """Represents the context of a request"""

    # Initializes the Context with the current socket and request
    def __init__(self, socket, request: Request):
        self._socket = socket
        self._request = request

    def get_req(self) -> Request:
        """Gets the request"""
        return self._request

    def write(self, response: Response):
        """Writes the response to the socket"""
        self._socket.send(response.to_bytes())
