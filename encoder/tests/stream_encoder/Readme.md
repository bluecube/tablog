# Stream encoder test

This is a helper to allow integration testing stream encoder and predictors
and its associated python decorder functions.

This test implements a minimalistic RPC-ish interface in which allows the python
test code to call pieces of stream encoder.

## The protocol

The binary communicates over stdin and stdout. Stderr is used to pass debug messages
to the test runner.
Exit codes other than zero indicate a failure.

Each input line is a single function to call. The line consits of function name,
optionally a type and remaining arguments, all separated by commas.
The type is encoded as `/[us](8|16|32|64)/`.

Each command outputs a record starting with a decimal length, newline and
binary output padded to a whole byte.
