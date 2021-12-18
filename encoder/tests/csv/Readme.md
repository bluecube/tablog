# CSV test

This test is a simple wrapper around Tablog encoder that compresses CSV file with data.

The CSV file is read from standard input and the compressed stream is written to
standard output.

First row of the CSV contains column names, second row contains types for each
column, encoded as `/[us](8|16|32|64)/`.
Following rows have numbers in decimal.

Because Tablog encoder fixes the column count and types at compile time, this
executable can only handle several combinations of types of the columns.
If unsupported types are selected, the process ends with error code 128.

All rows must have the same number of columns and all values must fit into the
column type.
