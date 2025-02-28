from config.config import Config
from controller.controller import Controller
from proto.context import Context
from proto.request import Request
from repository.ram_repository import RamRepository
from services.client_service import ClientService
from services.message_service import MessagesService

import selectors
import socket
import sys
import signal


class MessageUServer:
    def __init__(self, *, port, addr="localhost", backlog=100):
        self._sel = selectors.DefaultSelector()
        self._addr = addr
        self._port = port
        self._backlog = backlog
        self._sock = socket.socket()

        self._setup()
        self._install_sig_handler()
        self._setup_controller()

    def serve(self):
        print(f"Serving on port {self._port}...")
        while True:
            events = self._sel.select()
            for key, mask in events:
                cb = key.data
                cb(key.fileobj, mask)

    def _setup_controller(self):
        repo = RamRepository()

        self._controller = Controller(
            client_service=ClientService(repo), messages_service=MessagesService(repo)
        )

    def _setup(self):
        self._sock.bind((self._addr, self._port))
        self._sock.listen(self._backlog)
        self._sock.setblocking(False)
        self._sel.register(self._sock, selectors.EVENT_READ, self._accept)

    def _accept(self, sock, mask):
        conn, addr = sock.accept()
        print(f"Accepted {conn} from {addr}")
        conn.setblocking(False)
        self._sel.register(conn, selectors.EVENT_READ, self._read)

    def _read(self, conn, mask):
        try:
            data = conn.recv(1024)
            if data:
                self._controller.dispatch(conn, data)
        except Exception as e:
            print(f"{e}")
            self._sel.unregister(conn)
            conn.close()

    def _install_sig_handler(self):
        signal.signal(signal.SIGINT, self._sig_handler)

    def _sig_handler(self, sig, frame):
        print("Ctrl+C pressed, exisitng gracefully")
        self.shutdown()
        sys.exit(0)

    def shutdown(self):
        self._sel.close()
        self._sock.close()


def main():
    try:
        Config.load()
        server = MessageUServer(port=Config.PORT)

        server.serve()
    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
