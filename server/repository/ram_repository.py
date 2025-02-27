from repository.repository import Repository
from exceptions.exceptions import UniqueKeyViolationException


class RamRepository(Repository):
    def __init__(self):
        super().__init__()
        self._data = dict()

    def find_all(self):
        return list(self._data.items())

    def find_one(self, filter_cb):
        return dict(filter(filter_cb, self._data.items()))

    def create(self, id, obj):
        if self.find_one(lambda x: x.username == obj.username):
            raise UniqueKeyViolationException(f"Error: User with id={id} already exist")
        self._data[id] = obj
