from decoder import int_type

import dataclasses
import collections
import typing


@dataclasses.dataclass
class Dataset:
    name: str
    field_names: list[str]
    field_types: list[int_type.IntType]
    iter_callable: collections.abc.Callable[[], collections.abc.Iterator[list[int]]]
    length: typing.Optional[int]

    def __iter__(self):
        return self.iter_callable()


def show_content(datasets):
    for d in datasets:
        content = ", ".join(f"{n}({t})" for n, t in zip(d.field_names, d.field_types))
        print(f"{d.name}: {content}")
