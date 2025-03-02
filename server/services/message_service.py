from entities.message_entity import MessageEntity
from proto.request import SendMessagePayload
from repository.repository import Repository


class MessagesService:
    def __init__(self, repo: Repository):
        self._messages_repo = repo
        self._count = 0

    def create(self, sender_id, payload: SendMessagePayload) -> MessageEntity:
        msg = MessageEntity(
            self._count,
            sender_id,
            payload.client_id,
            payload.msg_type,
            payload.content,
        )
        self._count += 1
        self._messages_repo.save(msg.get_uuid(), msg)
        return msg

    def poll_msgs(self, client_id) -> list[MessageEntity]:
        msgs = self._messages_repo.find(
            lambda record: client_id == record[1].get_to_client()
        )
        return msgs
