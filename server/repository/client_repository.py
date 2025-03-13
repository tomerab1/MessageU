import sqlite3
from repository.repository import Repository
from entities.client_entity import ClientEntity


class ClientRepository(Repository):
    __tablename__ = "clients"

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
                    ID CHAR(16) NOT NULL PRIMARY KEY,
                    UserName CHAR(255) UNIQUE NOT NULL,
                    PublicKey CHAR(160) NOT NULL,
                    LastSeen DATETIME DEFAULT CURRENT_TIMESTAMP
                );
                """
            )
            conn.commit()

    def find_all(self):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT ID, UserName, PublicKey, LastSeen FROM clients")
            return [
                ClientEntity(row[0], row[1], row[2], row[3])
                for row in cursor.fetchall()
            ]

    def find(self, filter_cb):
        return list(filter(filter_cb, self.find_all()))

    def save(self, id: str, obj: ClientEntity):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                """
                INSERT INTO clients (ID, UserName, PublicKey, LastSeen) 
                VALUES (?, ?, ?, ?) 
                ON CONFLICT(ID) DO UPDATE SET 
                UserName=excluded.UserName, 
                PublicKey=excluded.PublicKey, 
                LastSeen=excluded.LastSeen
            """,
                (
                    id,
                    obj.get_username(),
                    obj.get_public_key(),
                    obj.get_last_seen().isoformat(),
                ),
            )
            conn.commit()

    def update_last_seen(self, uuid):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                f"UPDATE {self.__tablename__} SET LastSeen = CURRENT_TIMESTAMP WHERE ID = ?",
                (uuid,),
            )
            conn.commit()

    def delete(self, id: str):
        with sqlite3.connect(self._db_path) as conn:
            cursor = conn.cursor()
            cursor.execute("DELETE FROM clients WHERE id = ?", (id,))
            conn.commit()
