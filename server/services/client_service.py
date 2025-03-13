from repository.repository import Repository
from proto.request import RegistrationPayload
from entities.client_entity import ClientEntity
from exceptions.exceptions import UniqueKeyViolationException
import uuid


class ClientService:
    """Service layer for the client entity"""

    def __init__(self, repo: Repository):
        # inject the repository
        self._client_repo = repo

    def find_by_id(self, client_uuid: bytes) -> ClientEntity:
        """Find a client by its UUID"""
        result = self._client_repo.find(
            filter_cb=lambda user: client_uuid == user.get_uuid()
        )
        if not result:
            return None
        return next(iter(result))

    def find_all(self):
        """Find all clients"""
        return self._client_repo.find_all()

    def create(self, payload: RegistrationPayload):
        """Create a new client"""

        # if the username already exists, raise an exception
        if self._client_repo.find(
            filter_cb=lambda user: payload.username == user.get_username()
        ):
            raise UniqueKeyViolationException(
                f"Error: '{payload.username}' already exists"
            )

        new_uuid = uuid.uuid4().bytes
        user = ClientEntity(new_uuid, payload.username, payload.public_key)
        self._client_repo.save(new_uuid, user)
        return user
