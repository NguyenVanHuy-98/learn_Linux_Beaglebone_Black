#include <stdint.h>

#define PRU_SHARED_RAM   0x00010000u

typedef struct {
    volatile uint32_t magic;
    volatile uint32_t pwm_period;
    volatile uint32_t pwm_duty;
    volatile int32_t  enc0_count;
    volatile int32_t  enc1_count;
} pru_mailbox_t;

volatile pru_mailbox_t *mb = (volatile pru_mailbox_t*)PRU_SHARED_RAM;
/* PRU registers */
volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* simple delay (cycle based) */
static inline void delay_cycles(volatile uint32_t cycles)
{
    while (cycles--) {
        __asm__(" nop");
    }
}

/* Quadrature decoder (very common PRU pattern) */
static inline int8_t quad_decode(uint8_t prev, uint8_t curr)
{
    static const int8_t table[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };
    return table[(prev << 2) | curr];
}

int main(void)
{
    /* PWM parameters */
    uint32_t pwm_period = 20000 ;   // PRU cycles
    uint32_t pwm_duty   = 1000;    // initial duty

    /* Encoder counters */
    int32_t enc0_count = 0;
    int32_t enc1_count = 0;

    /* Previous encoder states */
    uint8_t enc0_prev = (__R31 >> 12) & 0x3;
    uint8_t enc1_prev = (__R31 >> 14) & 0x3;

    mb->magic = 0xBEEFCAFE;
    mb->pwm_period = pwm_period;
    mb->pwm_duty   = pwm_duty;
    mb->enc0_count = 0;
    mb->enc1_count = 0;

    while (1) {

        /* =====================
         * Read encoder inputs
         * ===================== */
        uint8_t enc0_curr = (__R31 >> 12) & 0x3; // R31_12,13
        uint8_t enc1_curr = (__R31 >> 14) & 0x3; // R31_14,15

        enc0_count += quad_decode(enc0_prev, enc0_curr);
        enc1_count += quad_decode(enc1_prev, enc1_curr);

        enc0_prev = enc0_curr;
        enc1_prev = enc1_curr;

        /* ======================================
         * Map encoder 0 to PWM duty (demo logic)
         * ====================================== */
        if (enc0_count >  100) enc0_count =  100;
        if (enc0_count < -100) enc0_count = -100;
        mb->enc0_count = enc0_count;
        mb->enc1_count = enc1_count;

        /* cho phép Linux set duty/period */
        if (mb->pwm_period != 0) pwm_period = mb->pwm_period;
        if (mb->pwm_duty   != 0) pwm_duty   = mb->pwm_duty;
        /* Duty from 5% → 95% */
        pwm_duty = (pwm_period * (enc0_count + 100)) / 200;
        if (pwm_duty < 100) pwm_duty = 100;
        if (pwm_duty > pwm_period - 100) pwm_duty = pwm_period - 100;

        /* =====================
         * Generate PWM on R30_0
         * ===================== */
        __R30 |=  (1u << 0);              // PWM HIGH
        delay_cycles(pwm_duty);

        __R30 &= ~(1u << 0);              // PWM LOW
        delay_cycles(pwm_period - pwm_duty);
    }

    return 0;
}
