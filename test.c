//#include <cstdio>
//#include <cstdint>
#include <stdio.h>
#include <stdint.h>
/*
*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
struct rumble_data {
    uint16_t padding_off: 2;
    uint16_t high_freq: 7;
    uint16_t high_amp: 7;

    uint16_t low_freq: 7;
    uint16_t low_amp: 7;
    uint16_t padding_on: 2;
    //uint16_t padding_other: 1;
} __attribute__((packed)) ;
#elif  __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
struct rumble_data {
    /*uint16_t padding_off: 2;
    uint16_t high_freq: 7;
    uint16_t high_amp: 7;
    uint16_t padding_on: 1;
    uint16_t low_amp: 7;
    uint16_t low_freq: 7;
    uint16_t padding_other: 1;*/
} __attribute__((packed)) ;
#endif

int main() {
    uint8_t high_freq = 0x40;
    uint8_t high_amp = 0x00;
    uint8_t low_freq = 0x40;
    uint8_t low_amp = 0x00;

    #if 0
    high_freq = 0x00;
    high_amp = 0x00;
    low_freq = 0x00;
    low_amp = 0x00;
    #endif 

    struct rumble_data r = {
        .padding_off = 0,
        .high_freq = high_freq,
        .high_amp = high_amp,

        .low_freq = low_freq,
        .low_amp = low_amp,
        .padding_on = 1,
        //.padding_other = 1,
    };

    uint8_t *data = (uint8_t *)(&r);
    //printf("%02x %02x %02x %02x ", data[0], data[1], data[2], data[3]);
    #if 0
    for (int i = r.high_freq; i < 0x7f; i+=1) {
        r.high_freq = i;
        printf("0x%02x 0x%02x 0x%02x 0x%02x ", data[0], data[1], data[2], data[3]);
        printf("\n");
    }
    #endif

    #if 0
    for (int i = r.low_freq; i < 0x7f; i+=1) {
        r.low_freq = i;
        printf("0x%02x 0x%02x 0x%02x 0x%02x ", data[0], data[1], data[2], data[3]);
        printf("\n");
    }
    #endif

    r.high_freq = 0;
    r.high_amp = 0;
    r.low_freq = 0;
    r.low_amp = 0;

    printf("0x%02x 0x%02x 0x%02x 0x%02x ", data[0], data[1], data[2], data[3]);

    uint32_t *a = (uint32_t *)(&r);
    printf("\n0x%08x\n", *a);

    printf("\n");
    return 0;
}
