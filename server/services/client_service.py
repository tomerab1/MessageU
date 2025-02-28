from repository.repository import Repository
from proto.request import RegistrationPayload
from entities.client_entity import ClientEntity
from exceptions.exceptions import UniqueKeyViolationException
import uuid


class ClientService:
    def __init__(self, repo: Repository):
        self._client_repo = repo

    def find_by_id(self, uuid) -> ClientEntity:
        (client, *_) = self._client_repo.find_one(
            filter_cb=lambda record: uuid == record[1].get_uuid()
        ).values()

        return client

    def find_all(self):
        return self._client_repo.find_all()

    def create(self, payload: RegistrationPayload):
        if self._client_repo.find_one(
            filter_cb=lambda record: payload.username == record[1].get_username()
        ):
            raise UniqueKeyViolationException(
                f"Error: '{payload.username}' already exist"
            )

        user = ClientEntity(uuid.uuid4().bytes, payload.username, payload.public_key)
        self._client_repo.save(user)
        return user
