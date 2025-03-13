from proto.request import MessageTypes


class MessageEntity:
    """A class to represent a message entity."""

    def __init__(self, id, from_client, to_client, msg_type, content):
        self._id = id
        self._from_client = from_client
        self._to_client = to_client
        self._msg_type = msg_type
        self._content = content

    def get_id(self):
        return self._id

    def set_id(self, id):
        self._id = id

    def get_from_client(self):
        return self._from_client

    def set_from_client(self, from_client):
        self._from_client = from_client

    def get_to_client(self):
        return self._to_client

    def set_to_client(self, to_client):
        self._to_client = to_client

    def get_msg_type(self) -> MessageTypes:
        return self._msg_type

    def set_msg_type(self, msg_type: MessageTypes):
        self._msg_type = msg_type

    def get_content(self):
        return self._content

    def set_content(self, content):
        self._content = content

    def __repr__(self):
        return f"Message({self._id}, {self._from_client}, {self._to_client})"
