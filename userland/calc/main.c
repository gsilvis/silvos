#include "userland.h"

void erase(unsigned int n) {
  do {
    putch('\177');
    n = n / 10;
  } while (n > 0);
}

void put(unsigned int n) {
  static char work[10];
  unsigned int index = 10;
  do {
    index--;
    work[index] = (n % 10) + '0';
    n = n / 10;
  } while (n > 0);
  for (; index < 10; index++) {
    putch(work[index]);
  }
}

static char input;
static char has_accum;
static char has_trailing_space = 0;

char operate(unsigned int accum, unsigned int* above) {
  if (has_trailing_space) {
    erase(0);
  }
  erase(accum);
  if (above == 0) {
    putch('X');
    return '\n';
  }
  erase(0);
  erase(*above);
  switch (input) {
    case '+':
      *above = (*above) + (accum);
      break;
    case '*':
      *above = (*above) * (accum);
      break;
    case '-':
      *above = (*above) - (accum);
      break;
    case '%':
      if (accum == 0) {
        putch('Z');
        return '\n';
      }
      *above = (*above) % (accum);
      break;
    case '/':
      if (accum == 0) {
        putch('Z');
        return '\n';
      }
      *above = (*above) / (accum);
      break;
    default:
      putch('E');
      return '\n';
  }
  put(*above);
  has_trailing_space = 0;
  /* This makes the frame above us print a space and go back to waiting for a
   * number / operator */
  return ' ';
}

char work(unsigned int* above) {
  unsigned int accum = 0;
  input = getch();
  has_accum = 0;
  while(1) {
    if ('0' <= input && input <= '9') {
      if (has_accum) {
        erase(accum);
      }
      accum = accum * 10 + (input - '0');
      put(accum);
      has_accum = 1;
      has_trailing_space = 0;
      input = getch();
      continue;
    }
    switch (input) {
      case '\n':
        return '\n';
      case ' ':
        if (has_accum) {
          if (!has_trailing_space) {
            putch(' ');
            has_trailing_space = 1;
          }
          /* Pass it up the stack! (to here) */
          input = work(&accum);
          has_accum = 1;
          /* Don't get a new input (we want to process what we got) */
          continue;
        }
        break;
      default:
        if (!has_accum) {
          if (above == 0) {
            putch('N');
            return '\n';
          }
          /* Pass it up the stack! (from here) */
          return input;
        }
        return operate(accum, above);
    }
    input = getch();
  }
}

void main() {
  while (1) {
    has_trailing_space = 0;
    has_accum = 0;
    work(0);
    putch('\n');
    putch('\r');
  }
}

