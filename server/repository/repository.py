from abc import ABC, abstractmethod


class Repository(ABC):
    @abstractmethod
    def find_all(self):
        pass

    @abstractmethod
    def find(self, filter_cb):
        pass

    @abstractmethod
    def save(self, id, obj):
        pass

    # @abstractmethod
    # def update(self, id, obj):
    #     pass

    # @abstractmethod
    # def delete(self, id):
    #     pass
