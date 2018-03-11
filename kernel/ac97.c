#include "ac97.h"

#include "alloc.h"
#include "com.h"
#include "memory-map.h"
#include "pagefault.h"
#include "pci.h"
#include "threads.h"
#include "util.h"

typedef struct {
  uint32_t buf_phys_addr;
  uint16_t buf_len;
  uint16_t flags;
} __attribute__((packed)) ac97_buffer_descriptor;

#define AC97_BUF_DESCR_FLAGS_BUFFER_UNDERRUN    0x4000
#define AC97_BUF_DESCR_FLAGS_IRQ_ON_COMPLETION  0x8000

#define AC97_NABMBAR_BUFDESC 0x10
#define AC97_NABMBAR_IX 0x14
#define AC97_NABMBAR_LVI 0x15
#define AC97_NABMBAR_SR 0x16
#define AC97_NABMBAR_CONTROL 0x1B

/* Status register flags */
#define AC97_X_SR_DCH   0x01  /* DMA controller halted */
#define AC97_X_SR_CELV  0x02  /* Current equals last valid */
#define AC97_X_SR_LVBCI 0x04  /* Last valid buffer completion interrupt */
#define AC97_X_SR_BCIS  0x08  /* Buffer completion interrupt status */
#define AC97_X_SR_FIFOE 0x10  /* FIFO error */

static uint16_t nambar;
static uint16_t nabmbar;
static ac97_buffer_descriptor* buffer_descriptors;
static uint16_t *(buffers[32]);

LIST_HEAD(ac97_wait);

void ac97_device_register (uint8_t bus, uint8_t device, uint8_t function) {
  nambar = pci_read(4, function, device, bus) & 0xFFFC;
  nabmbar = pci_read(5, function, device, bus) & 0xFFFC;
  outb(nabmbar+AC97_NABMBAR_CONTROL, 0x18); /* turn on interrupts */
  pci_write(1, function, device, bus, 5); /* enable busmastering */
  buffer_descriptors = allocate_phys_page();
  for (int i = 0; i < 32; i++) {
    buffers[i] = (uint16_t *)allocate_phys_page();
    buffer_descriptors[i].buf_phys_addr = virt_to_phys((uint64_t)(buffers[i]));
    buffer_descriptors[i].buf_len = 2048;
    /* Decrease the interrupt load somewhat by only waking up on every 8th
     * buffer. */
    if (i%8 == 0) {
      buffer_descriptors[i].flags = AC97_BUF_DESCR_FLAGS_IRQ_ON_COMPLETION;
    } else {
      buffer_descriptors[i].flags = 0;
    }
  }
  outd(nabmbar+AC97_NABMBAR_BUFDESC, virt_to_phys((uint64_t)buffer_descriptors)); /* load bufdesc table */
  outb(nabmbar+AC97_NABMBAR_LVI, 31);
}

int ac97_enqueue_audio_out (const void *samples) {
  static uint16_t started = 0;
  wait_event(ac97_wait, ((!started) || CIRC_SPACE(inb(nabmbar+AC97_NABMBAR_LVI), inb(nabmbar+AC97_NABMBAR_IX), 32)));
  uint8_t new_lvi = (inb(nabmbar+AC97_NABMBAR_LVI) + 1) % 32;
  int err = 0;
  if ((err = copy_from_user(buffers[new_lvi], samples, 4096))) {
    return err;
  }
  outb(nabmbar+AC97_NABMBAR_LVI, new_lvi);
  if (started == 0) {
    started = 1;
    outb(nabmbar+AC97_NABMBAR_CONTROL, 0x19);
  }
  return 0;
}

void ac97_handle_interrupt (void) {
  uint16_t sr = inw(nabmbar+AC97_NABMBAR_SR);
  if (sr & AC97_X_SR_LVBCI) {
    com_printf("Warning: last ac97 buffer complete\n");
    outw(nabmbar+AC97_NABMBAR_SR, AC97_X_SR_LVBCI);
  } else if (sr & AC97_X_SR_BCIS) {
    wake_up_all(ac97_wait);
    outw(nabmbar+AC97_NABMBAR_SR, AC97_X_SR_BCIS);
  } else if (sr & AC97_X_SR_FIFOE) {
    com_printf("Warning: ac97 buffer empty\n");
    outw(nabmbar+AC97_NABMBAR_SR, AC97_X_SR_FIFOE);
  } else {
    com_printf("Warning: ac97 interrupt with no meaning\n");
  }
  // com_printf("current index: %d\n", inb(nabmbar+AC97_NABMBAR_IX));
}
