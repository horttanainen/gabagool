"""CLI checks: python3 verify.py.

Requires Python 3 and the compiled gaba binary. Writes one scratch fixture
alongside this script; executes only the local interpreter.
"""

from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent
binary = root / "gaba"
fixture = root / "check.gool"
codes = dict(zip(">+.<-,[]", range(8)))


def encode(source):
    return "\n".join(
        " ".join("gool" if codes[char] & (1 << bit) else "gaba" for bit in (2, 1, 0))
        for char in source
    ).encode()


def run(path, data=b""):
    return subprocess.run([str(binary), str(path)], input=data, capture_output=True, timeout=5)


example = root / "gaba.gool"
words = example.read_bytes().split()
assert set(words) == {b"gaba", b"gool"}
assert len(words) % 3 == 0
result = run(example)
assert (result.returncode, result.stdout, result.stderr) == (0, (root / "tony.txt").read_bytes(), b"")

cases = [
    ("empty", b"", b"", b""),
    ("nested loops", encode("++[>++[>+<-]<-]>>."), b"", b"\x04"),
    ("skip nested loops", encode("[+[+]-]."), b"", b"\x00"),
    ("byte wrapping", encode("-.+."), b"", b"\xff\x00"),
    ("input and EOF", encode(",.,."), b"A", b"A\x00"),
    ("whitespace", encode("+.").replace(b" ", b"\t\r\n"), b"", b"\x01"),
]
for name, source, data, expected in cases:
    fixture.write_bytes(source)
    result = run(fixture, data)
    assert (result.returncode, result.stdout, result.stderr) == (0, expected, b""), name

errors = [
    ("unknown word", b"gaba gool nope", b"only gaba and gool"),
    ("joined words", b"gabagaba gool", b"separate gaba or gool"),
    ("embedded NUL", b"ga\0a gaba gaba", b"only gaba and gool"),
    ("long word", b"goolx gaba gool", b"separate gaba or gool"),
    ("old language", b"uuga uuga buuga", b"only gaba and gool"),
    ("incomplete instruction", b"gaba gool", b"incomplete instruction"),
    ("unmatched start", encode("["), b"unmatched loop start"),
    ("unmatched end", encode("]"), b"unmatched loop end"),
    ("left boundary", encode("<"), b"before start of memory"),
    ("right boundary", encode(">" * 30000), b"past end of memory"),
    ("program limit", encode("+" * 65537), b"exceeds 65536 instructions"),
]
for name, source, message in errors:
    fixture.write_bytes(source)
    result = run(fixture)
    assert result.returncode != 0 and message in result.stderr, name

fixture.unlink()
print(f"PASS: exact ASCII art, two-word source, and {len(cases) + len(errors)} CLI cases")
