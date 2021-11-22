# Stream encoder test

This is a helper to allow integration testing stream encoder and its associated python
decorder functions.

This test implements a minimalistic RPC-ish interface in which allows the python
test code to call pieces of stream encoder.

## The protocol

Each input line is a single function to call. The line consits of function name,
optionally a type and remaining arguments, all separated by commas.
The type is encoded as `/[us](8|16|32|64)/`.

Each command outputs a record starting with a decimal length, newline and
binary output padded to a whole byte.
