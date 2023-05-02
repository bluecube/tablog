import subprocess
import xml.etree.ElementTree
import pathlib

import pytest


def find_unit_tests():
    return pathlib.Path(__file__).parent.parent / "build" / "tests" / "unit" / "unit_tests"


class Catch2Test:
    @classmethod
    def collect_tests(cls, binary):
        xml = cls._xml_test_result(binary, "--list-tests")

        return [
            Catch2Test(binary, e.text)
            for e in xml.findall("./TestCase/Name")
        ]

    def __init__(self, binary, test_name):
        self.binary = binary
        self.test_name = test_name

    def __str__(self):
        return self.test_name

    def add_command(self, command: str):
        self._data.append(command)

    def run(self) -> None:
        self._xml_test_result(self.binary, self.test_name)
        # TODO: When a test fails, this just raises on the test binary
        # status code. This is reliable, but hides a lot of info from the test.
        # Most problematic: This completely hides the test rng seed.

    @staticmethod
    def _xml_test_result(binary, *more_args):
        command = [binary, "--reporter", "xml"]
        command.extend(more_args)
        run = subprocess.run(command, cwd=binary.parent, capture_output=True)
        run.check_returncode()
        return xml.etree.ElementTree.fromstring(run.stdout)


@pytest.mark.parametrize("test", Catch2Test.collect_tests(find_unit_tests()), ids=lambda t: str(t))
def test(test):
    test.run()


if __name__ == "__main__":
    print([str(x) for x in Catch2Test.collect_tests(find_unit_tests())])
