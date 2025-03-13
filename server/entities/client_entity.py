from datetime import datetime


class ClientEntity:
    """A class to represent a client entity."""

    def __init__(
        self, uuid: bytes, username: str, public_key: str, last_seen: datetime = None
    ):
        self._uuid = uuid
        self._username = username
        self._public_key = public_key
        self._last_seen = last_seen or datetime.now()

    def get_uuid(self):
        return self._uuid

    def set_uuid(self, uuid):
        self._uuid = uuid

    def get_username(self):
        return self._username

    def set_username(self, username):
        self._username = username

    def get_public_key(self):
        return self._public_key

    def set_public_key(self, public_key):
        self._public_key = public_key

    def get_last_seen(self):
        return self._last_seen

    def set_last_seen(self, last_seen):
        self._last_seen = last_seen

    def __repr__(self):
        uuid_hex = self._uuid.hex()
        return f"ClientEntity({uuid_hex}, {self._username}, {self._public_key}, {self._last_seen})"
