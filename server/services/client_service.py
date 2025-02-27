from repository.repository import Repository
from proto.request import RegistrationPayload
from entities.client_entity import ClientEntity
import uuid


class ClientService:
    def __init__(self, repo: Repository):
        self._client_repo = repo

    def find_by_id(self, uuid) -> ClientEntity:
        return self._client_repo.find_one(filter_cb=lambda record: uuid in record)

    def find_all(self):
        return self._client_repo.find_all()

    def create(self, payload: RegistrationPayload):
        user = ClientEntity(uuid.uuid4().bytes, payload.username, payload.public_key)
        self._client_repo.save(user)
        return user
