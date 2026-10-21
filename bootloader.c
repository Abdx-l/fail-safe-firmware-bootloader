#include "bootloader.h"
#include <string.h>

uint32_t image_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

void boot_init(boot_state *state, const uint8_t *factory_image, size_t length) {
    memset(state, 0, sizeof(*state));
    state->trial = -1;
    state->prior = -1;
    if (factory_image && length > 0 && length <= IMAGE_MAX) {
        memcpy(state->bank[0].bytes, factory_image, length);
        state->bank[0].length = length;
        state->bank[0].crc = image_crc32(factory_image, length);
        state->bank[0].valid = 1;
    }
}

int boot_stage(boot_state *state, const uint8_t *image, size_t length, uint32_t expected_crc,
               const uint8_t *signature, size_t signature_length,
               verify_signature_fn verify, void *context) {
    if (!state || !image || !signature || !verify || length == 0 || length > IMAGE_MAX ||
        signature_length == 0 || signature_length > SIGNATURE_MAX || state->trial != -1)
        return -1;
    if (image_crc32(image, length) != expected_crc) return -2;
    if (!verify(image, length, signature, signature_length, context)) return -3;
    int target = 1 - state->active;
    state->bank[target].valid = 0;
    memcpy(state->bank[target].bytes, image, length);
    state->bank[target].length = length;
    state->bank[target].crc = expected_crc;
    state->bank[target].valid = 1;
    state->prior = state->active;
    state->trial = target;
    state->generation++;
    return target;
}

int boot_select(boot_state *state) {
    if (!state) return -1;
    int candidate = state->trial >= 0 ? state->trial : state->active;
    image_bank *bank = &state->bank[candidate];
    if (!bank->valid || bank->length == 0 || image_crc32(bank->bytes, bank->length) != bank->crc) {
        if (state->trial >= 0) {
            boot_watchdog_reset(state);
            return state->bank[state->active].valid ? state->active : -1;
        }
        return -1;
    }
    return candidate;
}

void boot_confirm(boot_state *state) {
    if (state && state->trial >= 0) {
        state->active = state->trial;
        state->trial = -1;
        state->prior = -1;
    }
}

void boot_watchdog_reset(boot_state *state) {
    if (state && state->trial >= 0) {
        state->active = state->prior;
        state->trial = -1;
        state->prior = -1;
    }
}

