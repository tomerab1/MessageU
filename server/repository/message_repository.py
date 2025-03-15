import sqlite3
from repository.repository import Repository
from entities.message_entity import MessageEntity
from proto.request import MessageTypes


class MessageRepository(Repository):
    __tablename__ = "messages"

    def __init__(self, db_path):
        super().__init__()
        self._db_path = db_path
        self._ensure_table()

    def _ensure_table(self):
        with sqlite3.connect(self._db_path) as conn:
            conn.text_factory = bytes
            conn.executescript(
                f"""
                CREATE TABLE IF NOT EXISTS {self.__tablename__} (
                    ID INTEGER PRIMARY KEY AUTOINCREMENT,
                    ToClient CHAR(16) NOT NULL,
                    FromClient CHAR(16) NOT NULL,
                    Type CHAR(1) NOT NULL,
                    Content BLOB NOT NULL,
                    FOREIGN KEY (ToClient) REFERENCES clients(ID),
                    FOREIGN KEY (FromClient) REFERENCES clients(ID)
                )"""
            )
            conn.commit()

    def find_all(self):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                "SELECT ID, FromClient, ToClient, Type, Content FROM messages"
            )
            return [
                MessageEntity(
                    row[0],
                    row[1],
                    row[2],
                    MessageTypes.code_to_enum(int(row[3])),
                    row[4],
                )
                for row in cursor.fetchall()
            ]

    def find(self, filter_cb):
        return list(filter(filter_cb, self.find_all()))

    def save(self, id, obj: MessageEntity):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                f"""
                INSERT INTO {self.__tablename__} (ToClient, FromClient, Type, Content) 
                VALUES (?, ?, ?, ?) 
                """,
                (
                    obj.get_to_client(),
                    obj.get_from_client(),
                    obj.get_msg_type().value,
                    obj.get_content(),
                ),
            )
            conn.commit()
            return cursor.lastrowid

    def delete(self, id):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(f"DELETE FROM {self.__tablename__} WHERE ID=?", (id,))
            conn.commit()
