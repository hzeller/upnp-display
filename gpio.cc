// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
#include "gpio.h"

#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000
#define BCM2711_PERI_BASE        0xFE000000

#define GPIO_REGISTER_OFFSET     0x200000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define REGISTER_BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio_port_+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_port_+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

/*static*/ const uint32_t GPIO::kValidBits
= ((1 <<  2) | (1 <<  3) | (1 <<  4) | (1 <<  7)| (1 << 8) | (1 <<  9) |
   (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)| (1 <<17) | (1 << 18)|
   (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)| (1 << 27));

enum RaspberryPiModel {
  PI_MODEL_1,
  PI_MODEL_2,
  PI_MODEL_3,
  PI_MODEL_4
};

static int ReadFileToBuffer(char *buffer, size_t size, const char *filename) {
  const int fd = open(filename, O_RDONLY);
  if (fd < 0) return -1;
  ssize_t r = read(fd, buffer, size - 1); // assume one read enough
  buffer[r >= 0 ? r : 0] = '\0';
  close(fd);
  return r;
}

static RaspberryPiModel DetermineRaspberryModel() {
  char buffer[4096];
  if (ReadFileToBuffer(buffer, sizeof(buffer), "/proc/cpuinfo") < 0) {
    fprintf(stderr, "Reading cpuinfo: Could not determine Pi model\n");
    return PI_MODEL_3;  // safe guess fallback.
  }
  static const char RevisionTag[] = "Revision";
  const char *revision_key;
  if ((revision_key = strstr(buffer, RevisionTag)) == NULL) {
    fprintf(stderr, "non-existent Revision: Could not determine Pi model\n");
    return PI_MODEL_3;
  }
  unsigned int pi_revision;
  if (sscanf(index(revision_key, ':') + 1, "%x", &pi_revision) != 1) {
    fprintf(stderr, "Unknown Revision: Could not determine Pi model\n");
    return PI_MODEL_3;
  }

  // https://www.raspberrypi.org/documentation/hardware/raspberrypi/revision-codes/README.md
  const unsigned pi_type = (pi_revision >> 4) & 0xff;
  switch (pi_type) {
  case 0x00: /* A */
  case 0x01: /* B, Compute Module 1 */
  case 0x02: /* A+ */
  case 0x03: /* B+ */
  case 0x05: /* Alpha ?*/
  case 0x06: /* Compute Module1 */
  case 0x09: /* Zero */
  case 0x0c: /* Zero W */
    return PI_MODEL_1;

  case 0x04:  /* Pi 2 */
    return PI_MODEL_2;

  case 0x11: /* Pi 4 */
    return PI_MODEL_4;

  default:  /* a bunch of versions represneting Pi 3 */
    return PI_MODEL_3;
  }
}

static volatile uint32_t *mmap_bcm_register(off_t register_offset) {
  off_t base = BCM2709_PERI_BASE;  // safe fallback guess.
  switch (DetermineRaspberryModel()) {
  case PI_MODEL_1: base = BCM2708_PERI_BASE; break;
  case PI_MODEL_2: base = BCM2709_PERI_BASE; break;
  case PI_MODEL_3: base = BCM2709_PERI_BASE; break;
  case PI_MODEL_4: base = BCM2711_PERI_BASE; break;
  }

  int mem_fd = open("/dev/gpiomem", O_RDWR|O_SYNC);
  if (mem_fd < 0) {
    // Try fallback to old-school way.
    mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
  }
  if (mem_fd < 0)
    return NULL;

  uint32_t *result =
    (uint32_t*) mmap(NULL,                  // Any adddress in our space will do
                     REGISTER_BLOCK_SIZE,   // Map length
                     PROT_READ|PROT_WRITE,  // Enable r/w on GPIO registers.
                     MAP_SHARED,
                     mem_fd,                // File to map
                     base + register_offset // Offset to bcm register
                     );
  close(mem_fd);

  if (result == MAP_FAILED) {
    perror("mmap error: ");
    fprintf(stderr, "MMapping from base 0x%lx, offset 0x%lx\n",
            (long)base, (long)register_offset);
    return NULL;
  }
  return result;
}

static void busy_wait_nanos_rpi_1(long nanos) {
  if (nanos < 70) return;
  // The following loop is determined empirically on a 700Mhz RPi
  for (uint32_t i = (nanos - 70) >> 2; i != 0; --i) {
    asm("nop");
  }
}

static void busy_wait_nanos_rpi_2(long nanos) {
  if (nanos < 20) return;
  // The following loop is determined empirically on a 900Mhz RPi 2
  for (uint32_t i = (nanos - 20) * 100 / 110; i != 0; --i) {
    asm("");
  }
}

static void busy_wait_nanos_rpi_3(long nanos) {
  if (nanos < 20) return;
  for (uint32_t i = (nanos - 15) * 100 / 73; i != 0; --i) {
    asm("");
  }
}

static void busy_wait_nanos_rpi_4(long nanos) {
  if (nanos < 20) return;
  // Interesting, the Pi4 is _slower_ than the Pi3 ? At least for this busy loop
  for (uint32_t i = (nanos - 5) * 100 / 132; i != 0; --i) {
    asm("");
  }
}

// -- public interface
GPIO::GPIO() : output_bits_(0), gpio_port_(NULL) {
}

uint32_t GPIO::InitOutputs(uint32_t outputs) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init outputs but initialized.\n");
    return 0;
  }
  outputs &= kValidBits;   // Sanitize input.
  output_bits_ = outputs;
  for (uint32_t b = 0; b < 27; ++b) {
    if (outputs & (1 << b)) {
      INP_GPIO(b);   // for writing, we first need to set as input.
      OUT_GPIO(b);
    }
  }
  return output_bits_;
}

bool GPIO::Init() {
  const RaspberryPiModel model = DetermineRaspberryModel();
  gpio_port_ = mmap_bcm_register(GPIO_REGISTER_OFFSET);
  switch (model) {
  case PI_MODEL_1: busy_wait_impl_ = busy_wait_nanos_rpi_1; break;
  case PI_MODEL_2: busy_wait_impl_ = busy_wait_nanos_rpi_2; break;
  case PI_MODEL_3: busy_wait_impl_ = busy_wait_nanos_rpi_3; break;
  case PI_MODEL_4: busy_wait_impl_ = busy_wait_nanos_rpi_4; break;
  }
  return gpio_port_ != NULL;
}
