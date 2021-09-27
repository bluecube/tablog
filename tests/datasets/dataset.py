import dataclasses
import collections


@dataclasses.dataclass
class Dataset:
    name: str
    field_names: list[str]
    field_types: list[str]
    data_iterator: collections.abc.Iterator[list[int]]


def show_content(datasets):
    for d in datasets:
        content = ", ".join(f"{n}({t})" for n, t in zip(d.field_names, d.field_types))
        print(f"{d.name}: {content}")
