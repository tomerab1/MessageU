from repository.repository import Repository
from proto.request import RegistrationPayload
from entities.client_entity import ClientEntity
import uuid


class ClientService:
    def __init__(self, repo: Repository):
        self._client_repo = repo
        self._uuid = uuid.uuid4()

    def create(self, payload: RegistrationPayload):
        try:
            user = ClientEntity(self._uuid.bytes, payload.username, payload.public_key)
            self._client_repo.create(user.get_uuid(), user)
            return user
        except Exception as e:
            raise e
