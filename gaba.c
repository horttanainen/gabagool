#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_LIMIT 65536
#define TAPE_SIZE 30000

enum {
    MOVE_RIGHT, INCREMENT, OUTPUT, MOVE_LEFT,
    DECREMENT, INPUT, LOOP_START, LOOP_END
};

static unsigned char program[PROGRAM_LIMIT];
static size_t jumps[PROGRAM_LIMIT];
static size_t loop_stack[PROGRAM_LIMIT];
static uint8_t tape[TAPE_SIZE];

static void fail(const char *format, ...)
{
    va_list args;
    fputs("gaba: ", stderr);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

/* Return one binary digit, or -1 at EOF. Words must be separated. */
static int read_bit(FILE *source, size_t *line)
{
    int ch;
    while ((ch = fgetc(source)) != EOF && isspace((unsigned char)ch)) {
        if (ch == '\n') ++*line;
    }
    if (ch == EOF) return -1;

    const size_t word_line = *line;
    char word[5];
    size_t length = 0;
    do {
        if (length == sizeof(word) - 1) {
            fail("line %zu: expected a separate gaba or gool word", word_line);
        }
        word[length++] = (char)ch;
        ch = fgetc(source);
    } while (ch != EOF && !isspace((unsigned char)ch));
    if (ch == '\n') ++*line;
    word[length] = '\0';

    if (length == 4 && memcmp(word, "gaba", 4) == 0) return 0;
    if (length == 4 && memcmp(word, "gool", 4) == 0) return 1;
    fail("line %zu: only gaba and gool are allowed", word_line);
    return -1;
}

static size_t load_program(const char *path)
{
    FILE *source = fopen(path, "rb");
    if (!source) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    size_t line = 1, count = 0, depth = 0;
    unsigned int opcode = 0, words = 0;
    int bit;
    while ((bit = read_bit(source, &line)) != -1) {
        opcode = (opcode << 1) | (unsigned int)bit;
        if (++words < 3) continue;
        if (count == PROGRAM_LIMIT) fail("program exceeds %d instructions", PROGRAM_LIMIT);

        program[count] = (unsigned char)opcode;
        if (opcode == LOOP_START) {
            loop_stack[depth++] = count;
        } else if (opcode == LOOP_END) {
            if (depth == 0) fail("instruction %zu: unmatched loop end", count + 1);
            const size_t start = loop_stack[--depth];
            jumps[start] = count;
            jumps[count] = start;
        }
        ++count;
        opcode = 0;
        words = 0;
    }

    if (ferror(source)) fail("could not read %s", path);
    if (fclose(source) != 0) fail("could not close %s", path);
    if (words != 0) fail("incomplete instruction: each instruction needs three words");
    if (depth != 0) fail("instruction %zu: unmatched loop start", loop_stack[depth - 1] + 1);
    return count;
}

static void execute(size_t count)
{
    size_t pointer = 0;
    for (size_t pc = 0; pc < count; ++pc) {
        switch (program[pc]) {
        case MOVE_RIGHT:
            if (pointer + 1 == TAPE_SIZE) fail("instruction %zu: past end of memory", pc + 1);
            ++pointer;
            break;
        case INCREMENT:
            ++tape[pointer];
            break;
        case OUTPUT:
            if (putchar(tape[pointer]) == EOF) fail("could not write output");
            break;
        case MOVE_LEFT:
            if (pointer == 0) fail("instruction %zu: before start of memory", pc + 1);
            --pointer;
            break;
        case DECREMENT:
            --tape[pointer];
            break;
        case INPUT: {
            /* Flush prompts before waiting for input. EOF supplies zero. */
            if (fflush(stdout) == EOF) fail("could not write output");
            const int ch = getchar();
            if (ch == EOF && ferror(stdin)) fail("could not read input");
            tape[pointer] = ch == EOF ? 0 : (uint8_t)ch;
            break;
        }
        case LOOP_START:
            if (tape[pointer] == 0) pc = jumps[pc];
            break;
        case LOOP_END:
            if (tape[pointer] != 0) pc = jumps[pc];
            break;
        }
    }
    if (fflush(stdout) == EOF) fail("could not write output");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s program.gool\n", argv[0]);
        return EXIT_FAILURE;
    }
    execute(load_program(argv[1]));
    return EXIT_SUCCESS;
}
