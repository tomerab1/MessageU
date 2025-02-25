from abc import ABC, abstractmethod


class Repository(ABC):
    @abstractmethod
    def find(self, filter_cb):
        pass

    @abstractmethod
    def create(self, id, dto):
        pass

    @abstractmethod
    def update(self, id, dto):
        pass

    @abstractmethod
    def delete(self, id):
        pass
