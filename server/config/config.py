from dataclasses import dataclass


@dataclass
class Config:
    _PORT_PATH = "myport.info"
    PORT = 1357
    VERSION = 1

    def load():
        try:
            with open(Config._PORT_PATH) as f:
                txt = f.read()
            Config.PORT = int(txt)

        except FileNotFoundError:
            print(
                f"Error: '{Config._PORT_PATH}' does not exist, falling back to port {Config.PORT}"
            )
