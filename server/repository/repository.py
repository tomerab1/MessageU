from abc import ABC, abstractmethod


class Repository(ABC):
    """
    Abstract class for the repository pattern.
    Provides an interface for the repository classes to implement.
    """

    @abstractmethod
    def find_all(self):
        pass

    @abstractmethod
    def find(self, filter_cb):
        pass

    @abstractmethod
    def save(self, id, obj):
        pass

    @abstractmethod
    def delete(self, id):
        pass
