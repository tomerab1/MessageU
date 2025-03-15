from entities.message_entity import MessageEntity
from proto.request import SendMessagePayload
from repository.repository import Repository


class MessagesService:
    def __init__(self, repo: Repository):
        self._messages_repo = repo

    def create(self, sender_id, payload: SendMessagePayload) -> MessageEntity:
        msg = MessageEntity(
            None,
            sender_id,
            payload.client_id,
            payload.msg_type,
            payload.content,
        )
        msg_id = self._messages_repo.save(None, msg)
        msg.set_id(msg_id)
        return msg

    def poll_msgs(self, client_id) -> list[MessageEntity]:
        self._messages_repo.find(lambda msg: client_id == msg.get_to_client())
        msgs = self._messages_repo.find(lambda msg: client_id == msg.get_to_client())

        for msg in msgs:
            self._messages_repo.delete(msg.get_id())

        return msgs
