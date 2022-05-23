class TablogError(Exception):
    pass


class InputEmptyError(TablogError):
    pass


class UnsupportedVersionError(TablogError):
    pass
