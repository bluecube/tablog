import hypothesis

from .. import int_type


def int_types(unsigned_only=False):
    if unsigned_only:
        sign_strategy = hypothesis.strategies.just(False)
    else:
        sign_strategy = hypothesis.strategies.booleans()

    return hypothesis.strategies.builds(
        int_type.IntType,
        signed=sign_strategy,
        bitsize=hypothesis.strategies.sampled_from(int_type.IntType.allowed_bitsizes)
    )
