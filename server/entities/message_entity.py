class MessageEntity:
    """A class to represent a message entity."""

    def __init__(self, uuid, from_client, to_client, msg_type, content):
        self._uuid = uuid
        self._from_client = from_client
        self._to_client = to_client
        self._msg_type = msg_type
        self._content = content

    def get_uuid(self):
        return self._uuid

    def set_uuid(self, uuid):
        self._uuid = uuid

    def get_from_client(self):
        return self._from_client

    def set_from_client(self, from_client):
        self._from_client = from_client

    def get_to_client(self):
        return self._to_client

    def set_to_client(self, to_client):
        self._to_client = to_client

    def get_msg_type(self):
        return self._msg_type

    def set_msg_type(self, msg_type):
        self._msg_type = msg_type

    def get_content(self):
        return self._content

    def set_content(self, content):
        self._content = content

    def __repr__(self):
        return f"Message({self._uuid}, {self._from_client}, {self._to_client})"
