import hypothesis
from hypothesis.strategies import SearchStrategy

from .. import int_type


def int_types(unsigned_only=False) -> SearchStrategy[int_type.IntType]:
    """Returns a hypothesis strategy that produces int types."""

    if unsigned_only:
        sign_strategy = hypothesis.strategies.just(False)
    else:
        sign_strategy = hypothesis.strategies.booleans()

    return hypothesis.strategies.builds(
        int_type.IntType,
        signed=sign_strategy,
        bitsize=hypothesis.strategies.sampled_from(int_type.IntType.allowed_bitsizes),
    )


def int_type_values(int_type) -> SearchStrategy[int]:
    """Returns a hypothesis strategy that produces integers matching given int type"""
    r = int_type.range()
    return hypothesis.strategies.integers(min_value=r.start, max_value=r.stop - 1)


@hypothesis.strategies.composite
def typed_values(
    draw, unsigned_only=False
) -> SearchStrategy[tuple[int_type.IntType, int]]:
    """Returns a hypothesis strategy that returns a tuple of a type and a single
    value matching the type."""
    int_type = draw(int_types(unsigned_only=unsigned_only))
    v = draw(int_type_values(int_type))

    return (int_type, v)


@hypothesis.strategies.composite
def typed_lists(
    draw, min_size=0, max_size=None, unsigned_only=False
) -> SearchStrategy[tuple[int_type.IntType, list[int]]]:
    """Returns a hypothesis strategy that returns a tuple of a type and a list
    of values matching the type."""
    int_type = draw(int_types(unsigned_only=unsigned_only))
    lst = draw(
        hypothesis.strategies.lists(
            int_type_values(int_type), min_size=min_size, max_size=max_size
        )
    )

    return (int_type, lst)
