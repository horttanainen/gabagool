"""Run python3 encode.py to regenerate gaba.gool from the adjacent tony.txt.

Uses only Python's standard library and writes only gaba.gool.
"""

from pathlib import Path

root = Path(__file__).resolve().parent
art = (root / "tony.txt").read_bytes()
characters = sorted(set(art))
cells = {character: index for index, character in enumerate(characters)}
commands = []

# Initialize one byte cell for each distinct output character.
for index, character in enumerate(characters):
    if index:
        commands.append(">")
    commands.extend("+" * character)

pointer = max(0, len(characters) - 1)
for character in art:
    target = cells[character]
    difference = target - pointer
    commands.extend((">" if difference >= 0 else "<") * abs(difference))
    commands.append(".")
    pointer = target

if len(commands) > 65536:
    raise SystemExit("Encoded program exceeds the interpreter's 65,536-instruction limit")

codes = dict(zip(">+.<-,[]", range(8)))
lines = [
    " ".join("gool" if codes[command] & (1 << bit) else "gaba" for bit in (2, 1, 0))
    for command in commands
]
(root / "gaba.gool").write_text("\n".join(lines) + ("\n" if lines else ""), encoding="ascii")
print(f"Encoded {len(art)} output bytes into {len(lines)} instructions using only gaba and gool")
