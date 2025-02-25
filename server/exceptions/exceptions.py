class UniqueKeyViolationException(Exception):
    def __init__(self):
        super().__init__()


class NotFoundError(Exception):
    def __init__(self):
        super().__init__()
