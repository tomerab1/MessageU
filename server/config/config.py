from dataclasses import dataclass
import logging

logger = logging.getLogger(__name__)


@dataclass
class Config:
    _PORT_PATH = "myport.info"
    PORT = 1357
    VERSION = 2
    DATABASE_PATH = "defensive.db"
    REQ_HEADER_SZ = 23
    READ_SZ = 1024

    def load():
        try:
            with open(Config._PORT_PATH) as f:
                txt = f.read()
            Config.PORT = int(txt)

        except FileNotFoundError:
            logger.error(
                f"Error: '{Config._PORT_PATH}' does not exist, falling back to port {Config.PORT}"
            )
        except ValueError:
            logger.error(
                f"Error: '{Config._PORT_PATH}' does not contain a valid port number, falling back to port {Config.PORT}"
            )
