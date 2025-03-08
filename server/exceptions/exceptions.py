class UniqueKeyViolationException(Exception):
    """Exception for unique key violation"""

    def __init__(self, msg):
        super().__init__(msg)


class NotFoundError(Exception):
    """Exception for not found error"""

    def __init__(self, msg):
        super().__init__(msg)


class InvalidCodeError(Exception):
    """Exception for invalid code"""

    def __init__(self, msg):
        super().__init__(msg)


class InvalidPayloadError(Exception):
    """Exception for invalid payload"""

    def __init__(self, msg):
        super().__init__(msg)


class InvalidMessageTypeError(Exception):
    """Exception for invalid message type"""

    def __init__(self, msg):
        super().__init__(msg)


class InvalidPayloadResponseError(Exception):
    """Exception for invalid payload response"""

    def __init__(self, msg):
        super().__init__(msg)


class InvalidUUID(Exception):
    """Exception for invalid UUID"""

    def __init__(self, msg):
        super().__init__(msg)
