from config.config import Config


def main():
    Config.load()

    print(Config.PORT)


if __name__ == "__main__":
    main()
