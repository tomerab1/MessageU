from repository.repository import Repository


class RamRepository(Repository):
    def __init__(self):
        super().__init__()
        self._data = dict()

    def find_all(self):
        return list(self._data.values())

    def find(self, filter_cb):
        return dict(filter(filter_cb, self._data.items()))

    def save(self, id, obj):
        self._data[id] = obj
