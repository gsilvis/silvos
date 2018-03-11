#include "kbd.h"

#include "util.h"
#include "threads.h"
#include "vga.h"

/* Mapping from scancodes to ASCII characters */
const char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
/* As above, but for when shift is held */
const char scancode_caps[128] = "\0\e!@#$%^&*()_+\177\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0\\ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


char buf[256];
int buf_head;
int buf_tail;

char lshift;
char rshift;

LIST_HEAD(kbd_wait);

void init_kbd (void) {
  buf_head = 0;
  buf_tail = 0;
  lshift = 0;
  rshift = 0;
}

void read_key (void) {
  unsigned char c = inb(0x60);
  char r; /* Character to enqueue */
  /* Process input */
  if ((c & 0x7f) == 0x2a) {
    /* LShift down or up, depending on high bit */
    lshift = !(c & 0x80);
    return;
  }
  if ((c & 0x7f) == 0x36) {
    /* RShift down or up, depending on high bit */
    rshift = !(c & 0x80);
    return;
  }
  if (c & 0x80) {
    /* Not a normal character */
    return;
  }
  if (lshift || rshift) {
    /* Capitalize */
    r = scancode_caps[c];
  } else {
    /* Don't */
    r = scancode[c];
  }
  /* Put the character in the buffer */
  buf[buf_head] = r;
  buf_head = (buf_head + 1) & 255;
  if (CIRC_SPACE(buf_head, buf_tail, 256) == 0) {
    /* Buffer's full, so drop least-recent char. */
    buf_tail = (buf_tail + 1) & 255;
  }
  /* Wake a up a thread.  This causes a race condition, if there were two
   * threads waiting, and two characters came in close to eachother.  This should
   * be fixed at some point. */
  wake_up(kbd_wait);
}


char getch (void) {
  wait_event(kbd_wait, CIRC_CNT(buf_head, buf_tail, 256));
  char r = buf[buf_tail];
  buf_tail = (buf_tail + 1) % 256;
  return r;
}
