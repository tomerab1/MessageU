class UniqueKeyViolationException(Exception):
    def __init__(self):
        super().__init__()


class NotFoundError(Exception):
    def __init__(self):
        super().__init__()


class InvalidCodeError(Exception):
    def __init__(self, msg):
        super().__init__(msg)


class InvalidPayloadError(Exception):
    def __init__(self):
        super().__init__()
