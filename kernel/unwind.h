#ifndef __SILVOS_UNWIND_H
#define __SILVOS_UNWIND_H

#include <stdint.h>

void test_parse_eh_frame();
void gen_backtrace(uint64_t rsp, uint64_t rip, uint64_t rbp);

#endif
