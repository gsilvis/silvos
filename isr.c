#include "util.h"

void dumb_isr (void) {
  panic("wefwefwefwe");
}

void doublefault_isr (void) {
  panic("DOUBLE FAULT");
}

void isr_other (void) {
  panic("SOME ISR");
}

void isr_20 (void) {
  panic("ISR 20!");
}

void isr_21 (void) {
  panic("ISR 21!");
}
