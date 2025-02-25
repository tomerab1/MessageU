from repository.ram_repository import RamRepository


class ClientRamRepository(RamRepository):
    def __init__(self):
        super().__init__()

    def find(self, filter_cb):
        return super().find(filter_cb)

    def create(self, dto):
        raise NotImplementedError

    def update(self, id, dto):
        raise NotImplementedError

    def delete(self, id):
        raise NotImplementedError
