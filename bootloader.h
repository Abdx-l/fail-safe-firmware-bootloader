#ifndef BOOTLOADER_H
#define BOOTLOADER_H
#include <stddef.h>
#include <stdint.h>

#define IMAGE_MAX 4096
#define SIGNATURE_MAX 128

typedef int (*verify_signature_fn)(const uint8_t *image, size_t length,
                                   const uint8_t *signature, size_t signature_length,
                                   void *context);
typedef struct {
    uint8_t bytes[IMAGE_MAX];
    size_t length;
    uint32_t crc;
    int valid;
} image_bank;
typedef struct {
    image_bank bank[2];
    int active;
    int trial;
    int prior;
    uint32_t generation;
} boot_state;

uint32_t image_crc32(const uint8_t *data, size_t length);
void boot_init(boot_state *state, const uint8_t *factory_image, size_t length);
int boot_stage(boot_state *state, const uint8_t *image, size_t length, uint32_t expected_crc,
               const uint8_t *signature, size_t signature_length,
               verify_signature_fn verify, void *context);
int boot_select(boot_state *state);
void boot_confirm(boot_state *state);
void boot_watchdog_reset(boot_state *state);
#endif

