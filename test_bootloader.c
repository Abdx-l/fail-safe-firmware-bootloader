#include "bootloader.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int verifier(const uint8_t *image, size_t length, const uint8_t *signature,
                    size_t signature_length, void *context) {
    (void)image; (void)length; (void)context;
    return signature_length == 2 && signature[0] == 'O' && signature[1] == 'K';
}

int main(void) {
    const uint8_t factory[] = "factory";
    const uint8_t update[] = "version-2";
    const uint8_t accepted[] = "OK", rejected[] = "NO";
    boot_state state;
    boot_init(&state, factory, sizeof(factory));
    assert(image_crc32((const uint8_t *)"123456789", 9) == 0xcbf43926u);
    assert(boot_select(&state) == 0);
    assert(boot_stage(&state, update, sizeof(update), 0, accepted, 2, verifier, NULL) == -2);
    assert(boot_stage(&state, update, sizeof(update), image_crc32(update, sizeof(update)), rejected, 2, verifier, NULL) == -3);
    assert(boot_stage(&state, update, sizeof(update), image_crc32(update, sizeof(update)), accepted, 2, verifier, NULL) == 1);
    assert(boot_select(&state) == 1);
    boot_watchdog_reset(&state);
    assert(boot_select(&state) == 0);
    assert(boot_stage(&state, update, sizeof(update), image_crc32(update, sizeof(update)), accepted, 2, verifier, NULL) == 1);
    boot_confirm(&state);
    assert(boot_select(&state) == 1);
    state.bank[1].bytes[0] ^= 1;
    assert(boot_select(&state) == -1);
    puts("bootloader scenarios passed");
}

