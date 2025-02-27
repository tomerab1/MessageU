from repository.repository import Repository


class RamRepository(Repository):
    _auto_inc = 0

    def __init__(self):
        super().__init__()
        self._data = dict()

    def find_all(self):
        return list(self._data.values())

    def find_one(self, filter_cb):
        return dict(filter(filter_cb, self._data.items()))

    def save(self, obj):
        self._data[RamRepository._auto_inc] = obj
        RamRepository._auto_inc += 1
