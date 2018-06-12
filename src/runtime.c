#include <stdio.h>
#include <stdlib.h>

#define TAPE_WIDTH 64

#define ERROR_BRACKETS "\033[1;31mError:\033[0m Miss-matched bracket on line %d.\n"
#define ERROR_FILE "\033[1;31mError:\033[0m Could not read file \"%s\".\n"
#define ERROR_MEMORY "\033[1;31mError:\033[0m Insufficient memory.\n"
#define NOTICE_FLAG "\033[1;34mNotice:\033[0m Command line flag \"%s\" not recognized.\n"
#define VERSION "Version 1.1\nAuthor: Tilman Roeder\n"
#define HELP "usage: brainfuck [-himv] filename\n"\
             "       h : Print this help message\n"\
             "       i : Print data from tape as base 10 numbers\n"\
             "       m : Print memory after script halts\n"\
             "       v : Print interpreter version\n"


typedef struct tape {
  char data[TAPE_WIDTH];
  struct tape *prev;
  struct tape *next;
} Tape;

typedef enum {
  Left, Right,
} Direction;

typedef enum {
  Char, Int,
} PrintStyle;


// Reads a given source file and prints any
// errors (miss-matched []) to the standard output
char* readFile(char *file_name) {
  // Open file and allocate buffer
  FILE *file = fopen(file_name, "r");
  if (file == NULL || ferror(file)) {
    printf(ERROR_FILE, file_name);
    fclose(file);
    return NULL;
  }
  long len = 0;
  fseek(file, 0, SEEK_END);
  len = ftell(file);
  char *buffer = malloc(len * sizeof(char));
  if (buffer == NULL) {
    printf(ERROR_MEMORY);
    fclose(file);
    return NULL;
  }
  // Read file contents to temp buffer
  fseek(file, 0, SEEK_SET);
  if (len != fread(buffer, 1, len, file)) {
    printf(ERROR_FILE, file_name);
    free(buffer);
    fclose(file);
    return NULL;
  }
  // Count number of instructions and check the code is valid
  long inst_count = 0;
  int line = 1;
  int line_last_open = 0;
  int line_last_close = 0;
  int open_loops = 0;
  for (int i = 0; i < len; i ++) {
    switch (buffer[i]) {
      case '[':
      case ']':
        if (buffer[i] == '[') {
          open_loops ++;
          line_last_open = line;
        } else {
          open_loops --;
          line_last_close = line;
        }
        // Catch brackets that are the wrong way around
        if (open_loops < 0) {
          printf(ERROR_BRACKETS, line_last_close);
          free(buffer);
          fclose(file);
          return NULL;
        }
        // fallthrough
      case '>':
      case '<':
      case '+':
      case '-':
      case '.':
      case ',':
        inst_count ++;
        break;
      case '\n':
        line ++;
        break;
    }
  }
  if (open_loops != 0) {
    printf(ERROR_BRACKETS, open_loops > 0 ? line_last_open : line_last_close);
    free(buffer);
    fclose(file);
    return NULL;
  }
  // Write instructions to script
  char *script = malloc((inst_count + 1) * sizeof(char));
  if (script == NULL) {
    printf(ERROR_MEMORY);
    free(buffer);
    fclose(file);
    return NULL;
  }
  for (int i = 0, j = 0; i < len; i ++) {
    switch (buffer[i]) {
      case '>':
      case '<':
      case '+':
      case '-':
      case '[':
      case ']':
      case '.':
      case ',':
        script[j] = buffer[i];
        j ++;
        break;
    }
  }
  script[inst_count] = '\0';
  return script;
}

Tape* makeTape() {
  Tape *tape = malloc(sizeof(Tape));
  if (tape != NULL) {
    // Init tape
    for (int i = 0; i < TAPE_WIDTH; i ++) {
      tape->data[i] = (char)0;
    }
    tape->prev = NULL;
    tape->next = NULL;
  }
  return tape;
}
void destroyTape(Tape *tape) {
  if (tape == NULL) {
    return;
  }
  while(tape->prev != NULL) {
    tape = tape->prev;
  }
  Tape *prev;
  while (tape->next != NULL) {
    prev = tape;
    tape = tape->next;
    free(prev);
  }
  free(tape);
}
int seekTape(Tape **tape, int *pos, Direction dir) {
  switch (dir) {
    case Left:
      *pos = *pos - 1;
      if (*pos < 0 && (*tape)->prev == NULL) {
        Tape *ext = makeTape();
        if (ext == NULL) return 0;
        *pos = TAPE_WIDTH - 1;
        (*tape)->prev = ext;
        ext->next = *tape;
        *tape = ext;
      } else if (*pos < 0) {
        *pos = TAPE_WIDTH - 1;
        *tape = (*tape)->prev;
      }
      break;
    case Right:
      *pos = *pos + 1;
      if (*pos >= TAPE_WIDTH && (*tape)->next == NULL) {
        Tape *ext = makeTape();
        if (ext == NULL) return 0;
        *pos = 0;
        (*tape)->next = ext;
        ext->prev = *tape;
        *tape = ext;
      } else if (*pos >= TAPE_WIDTH) {
        *pos = 0;
        *tape = (*tape)->next;
      }
      break;
  }
  // Success
  return 1;
}
void printTape(Tape *tape) {
  while(tape->prev != NULL) {
    tape = tape->prev;
  }
  printf("START OF TAPE\n");
  while(tape != NULL) {
    for (int i = 0; i < TAPE_WIDTH; i ++) {
      printf("[%d]", tape->data[i]);
    }
    printf("\n");
    tape = tape->next;
  }
  printf("END OF TAPE\n");
}

void run(char *script, Tape *tape, PrintStyle print_style) {
  int i = 0;
  int pos = 0;
  while (script[i] != '\0') {
    switch (script[i]) {
      case '>':
        if (!seekTape(&tape, &pos, Right)) {
          printf(ERROR_MEMORY);
          return;
        }
        i ++;
        break;
      case '<':
        if (!seekTape(&tape, &pos, Left)) {
          printf(ERROR_MEMORY);
          return;
        }
        i ++;
        break;
      case '+':
        tape->data[pos] ++;
        i ++;
        break;
      case '-':
        tape->data[pos] --;
        i ++;
        break;
      case '.':
        if (print_style == Int) {
          printf("%d ", (int)tape->data[pos]);
        } else {
          putchar(tape->data[pos]);
        }
        i ++;
        break;
      case ',':
        tape->data[pos] = getchar();
        i ++;
        break;
      case '[':
        if (tape->data[pos] != (char)0) {
          i ++;
        } else {
          int open_loops = 1;
          i ++;
          // NOTE: We expect a valid script and don't check for \0 char (is checked in readFile)
          while (open_loops > 0) {
            if (script[i] == '[') {
              open_loops ++;
            } else if (script[i] == ']') {
              open_loops --;
            }
            i ++;
          }
        }
        break;
      case ']':
        if (tape->data[pos] == (char)0) {
          i ++;
        } else {
          int open_loops = 1;
          i --;
          // NOTE: We expect a valid script and don't check for \0 char (is checked in readFile)
          while (open_loops > 0) {
            if (script[i] == '[') {
              open_loops --;
            } else if (script[i] == ']') {
              open_loops ++;
            }
            i --;
          }
          i += 2;
        }
        break;
    }
  }
}


int main(int argc, char *argv[]) {
  // Accept arguments and modifiers
  char *file = NULL;
  int m_debug = 0;
  int m_print_int = 0;
  int m_version = 0;
  int m_help = 0;
  for (int i = 1; i < argc; i ++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'm':
          m_debug = 1;
          break;
        case 'i':
          m_print_int = 1;
          break;
        case 'v':
          m_version = 1;
          break;
        case 'h':
          m_help = 1;
          break;
        default:
          printf(NOTICE_FLAG, argv[i]);
          break;
      }
    } else {
      file = argv[i];
      break;
    }
  }

  if (m_version) {
    printf(VERSION);
    return 0;
  } else if (m_help) {
    printf(VERSION);
    printf("\n");
    printf(HELP);
    return 0;
  } else if (file == NULL) {
    printf(HELP);
    return 0;
  }

  // Read the file and run the interpreter
  char *script = readFile(file);
  if (script != NULL) {
    Tape *tape = makeTape();
    if (tape == NULL) {
      printf(ERROR_MEMORY);
    } else {
      run(script, tape, m_print_int ? Int : Char);
      printf("\n");
      // Debug mode, print tape state after halt
      if (m_debug) printTape(tape);
      destroyTape(tape);
    }

    // Clean up
    free(script);
  }

  return 0;
}