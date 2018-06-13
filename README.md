# Brainfuck
A minimal brainfuck interpreter written in C. This implements the brainfuck language as
described in [the relevant Wikipedia article](https://en.wikipedia.org/wiki/Brainfuck). The folder
examples contains two example scripts.

Brainfuck was originally created by Urban Müller in 1993 and can be thought of as a basic Turing Machine.

## Building the interpreter
To build this interpreter, it should be sufficient to run `make` in the projects root directory. This will
create a binary called `brainfuck` in the root directory.

## Brainfuck
Brainfuck is a very simple language that consists of only 8 instructions and an infinite tape of memory,
all of which is initialized to `0`. 'The pointer' initially points to a position in this tape. The available
instructions are:

* `>` Move the pointer to the right
* `<` Move the pointer to the left
* `+` Increment the value at the pointer
* `-` Decrement the value at the pointer
* `.` Output the current value as a char
* `,` Read a char to the current value
* `[` If the value at the pointer is `0`, jump to matching `]`
* `]` If the value at the pointer is not `0`, jump to matching `[`

## Implementation
This interpreter implements a memory tape that is limited in length only by the available ram. Each value
in memory is stored a c `char` (8-bit / 1-byte / 0-255). When a script is loaded, all non instruction characters are discarded.
The I/O instructions use the standard in- and out-put.

## Additional features
### Syntax checking
When a script is loaded, the interpreter will alert the user of any miss-matched brackets.

### Interger output
The command line option `-i` (e.g. `brainfuck -i my-calculator.bf`) causes the interpreter to output the interger
value at the pointer as a decimal number.

### Memory dumps
When called with the option `-m` (e.g. `brainfuck -m my-script.bf`), the interpreter will output the
memory state once the script halts.
