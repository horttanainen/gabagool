# Gaba Gool
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/13fa4901-57f0-4aca-bdd4-4fad6eafa762" />

A small language whose source contains only `gaba`, `gool`, and whitespace.
`gaba.gool` prints a happy Tony Soprano ASCII portrait.

To build from source on macOS or Linux with a C compiler:

```sh
cc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror gaba.c -o gaba
./gaba gaba.gool
```

Every three words form one instruction. Newlines are just whitespace;
`gaba.gool` puts one instruction on each line. Comments are not allowed.

| Words | Meaning |
| --- | --- |
| `gaba gaba gaba` | Move one memory cell right |
| `gaba gaba gool` | Increase current cell by 1 |
| `gaba gool gaba` | Print current cell as one byte (ASCII for this example) |
| `gaba gool gool` | Move one memory cell left |
| `gool gaba gaba` | Decrease current cell by 1 |
| `gool gaba gool` | Read one byte into current cell (EOF sets it to zero) |
| `gool gool gaba` | Start loop; skip to after its end when current cell is zero |
| `gool gool gool` | End loop; repeat its body while current cell is nonzero |

This is a Brainfuck-style machine with 30,000 initially zero byte cells.
The pointer starts at the first cell. Values wrap between 0 and 255;
moving outside memory is an error. Nested loops are supported. Programs
may contain at most 65,536 instructions. Syntax is checked before execution.

The image is encoded entirely in `gaba.gool`. The interpreter does not read
`tony.txt` or have a built-in image; it executes the instructions in `gaba.gool`.
The example initializes one memory cell per distinct character, then moves
between those cells to print each character, including spaces and newlines.

`tony.txt` is the human-readable expected output. With Python 3 installed:

```sh
python3 encode.py     # Regenerate gaba.gool from tony.txt
python3 verify.py     # Compare exact output and check interpreter behavior
```
