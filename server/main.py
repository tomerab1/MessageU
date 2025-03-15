from config.config import Config
from controller.controller import Controller
from repository.client_repository import ClientRepository
from repository.message_repository import MessageRepository
from services.client_service import ClientService
from services.message_service import MessagesService
from proto.request import Request

import selectors
import socket
import sys
import signal
import logging

logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
)

logger = logging.getLogger(__name__)


class MessageUServer:
    """Represents the server for the MessageU app"""

    def __init__(self, *, port, addr="localhost", backlog=100):
        self._sel = selectors.DefaultSelector()
        self._addr = addr
        self._port = port
        self._backlog = backlog
        self._sock = socket.socket()
        self._buffers = dict()

        self._setup()
        self._install_sig_handler()
        self._setup_controller()

    def serve(self):
        """ "Starts the server and listens for incoming connections"""
        logger.info(f"Serving on port {self._port}...")
        while True:
            events = self._sel.select(timeout=0.1)
            for key, mask in events:
                cb = key.data
                cb(key.fileobj, mask)

    def _setup_controller(self):
        """Initializes the controller with the required services"""
        self._controller = Controller(
            client_service=ClientService(ClientRepository(Config.DATABASE_PATH)),
            messages_service=MessagesService(MessageRepository(Config.DATABASE_PATH)),
        )

    def _setup(self):
        """Sets up the server socket and registers the accept callback"""
        self._sock.bind((self._addr, self._port))
        self._sock.listen(self._backlog)
        self._sock.setblocking(False)
        self._sel.register(self._sock, selectors.EVENT_READ, self._accept)

    def _accept(self, sock, mask):
        """Accepts incoming connections"""
        conn, addr = sock.accept()
        logger.info(f"Accepted {conn} from {addr}")
        conn.setblocking(False)
        self._sel.register(conn, selectors.EVENT_READ, self._read)

    def _read(self, conn, mask):
        """Reads incoming data from the connection"""
        try:
            # We get the buffer of the current connection
            buffer = self._buffers.get(conn, b"")

            # Read the data until there is no more data to read
            while True:
                try:
                    chunk = conn.recv(Config.READ_SZ)
                    if not chunk:
                        break
                    buffer += chunk
                except BlockingIOError:
                    break

            # Update the buffer
            self._buffers[conn] = buffer
            # If we dont have enough data to read the header, return
            if len(buffer) < Config.REQ_HEADER_SZ:
                return

            # Parse the header.
            header = Request.Header.from_bytes(buffer[: Request._HEADER_SZ])
            total_length = Config.REQ_HEADER_SZ + header.payload_sz
            if len(buffer) < total_length:
                return

            # Extract the data from the buffer
            data = buffer[:total_length]
            # Advance the buffer (maybe there is more data)
            self._buffers[conn] = buffer[total_length:]
            self._controller.dispatch(conn, data)
        except Exception as e:
            logger.exception(f"{e}")
            self._sel.unregister(conn)
            if conn in self._buffers:
                del self._buffers[conn]
            conn.close()

    def _install_sig_handler(self):
        """Setup the sig handler for SIGINT"""
        signal.signal(signal.SIGINT, self._sig_handler)

    def _sig_handler(self, sig, frame):
        """Handles the SIGINT signal by releasing resources and closing connections"""
        logger.info("Ctrl+C pressed, exisitng gracefully")
        self.shutdown()
        sys.exit(0)

    def shutdown(self):
        """Releases resources and closes connections"""
        self._sel.close()
        self._sock.close()


def main():
    try:
        Config.load()
        server = MessageUServer(port=Config.PORT)
        server.serve()
    except Exception as e:
        logger.exception(e)
        logger.error(repr(e))


if __name__ == "__main__":
    main()
