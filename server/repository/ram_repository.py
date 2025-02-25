from repository.repository import Repository
from exceptions.exceptions import UniqueKeyViolationError, NotFoundError


class RamRepository(Repository):
    def __init__(self):
        super().__init__()
        self._data = dict()

    def find(self, filter_cb):
        return dict(filter(filter_cb, self._data.items()))

    def create(self, id, obj):
        if self.find(id):
            raise UniqueKeyViolationError()
        self._data[id] = obj

    def update(self, id, obj):
        if not self.find(id):
            raise NotFoundError()
        self._data[id] = obj

    def delete(self, id):
        if not self.find(id):
            raise NotFoundError()
        del self._data[id]
