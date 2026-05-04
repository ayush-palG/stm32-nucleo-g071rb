#include <stdint.h>

// -----------------------------------------------------------------------------
// Ring Buffer
// -----------------------------------------------------------------------------

// Size must be of power of 2, which allows masking instead of modulo
#define BUFFER_SIZE  16U
#define BUFFER_MASK  (BUFFER_SIZE - 1U)

typedef struct {
    volatile uint8_t bytes[BUFFER_SIZE];
    volatile uint32_t head;    // Written by ISR
    volatile uint32_t tail;    // Read by main
    volatile uint32_t count;
} RingBuffer;

static volatile RingBuffer rx_buffer = {0};

uint8_t rb_insert(uint8_t value)
{
    if (rx_buffer.count == BUFFER_SIZE) return 0;

    rx_buffer.bytes[rx_buffer.head++] = value;
    rx_buffer.head &= BUFFER_MASK;

    rx_buffer.count++;
    return 1;
}

uint8_t rb_fetch(uint8_t *value)
{
    if (rx_buffer.count == 0) return 0;

    *value = rx_buffer.bytes[rx_buffer.tail++];
    rx_buffer.tail &= BUFFER_MASK;

    rx_buffer.count--;
    return 1;
}

// -----------------------------------------------------------------------------
// RCC
// -----------------------------------------------------------------------------
#define RCC_BASE     0x40021000UL
#define RCC_IOPENR   (*(volatile uint32_t *) (RCC_BASE + 0x34))
#define RCC_APBENR1  (*(volatile uint32_t *) (RCC_BASE + 0x3C))

// -----------------------------------------------------------------------------
// GPIO
// -----------------------------------------------------------------------------
typedef struct {
    volatile uint32_t MODER;    // 0x00 Mode Register
    volatile uint32_t OTYPER;   // 0x04 Output Type Register
    volatile uint32_t OSPEEDR;  // 0x08 Output Speed Register
    volatile uint32_t PUPDR;    // 0x0C Pull-Up/Pull-Down Register
    volatile uint32_t IDR;      // 0x10 Input Data Register
    volatile uint32_t ODR;      // 0x14 Output Data Regsiter
    volatile uint32_t BSRR;     // 0x18 Bit Set/Reset Register
    volatile uint32_t LCKR;     // 0x1C Lock Configuration Register
    volatile uint32_t AFRL;     // 0x20 Alternate Function Low Register
    volatile uint32_t AFRH;     // 0x24 Alternate Function High Register
    volatile uint32_t BRR;      // 0x28 Bit Reset Register
} GPIO_t;

#define GPIOA_BASE  0x50000000UL
#define GPIOA       ((GPIO_t *) GPIOA_BASE)

// -----------------------------------------------------------------------------
// SysTick
// -----------------------------------------------------------------------------

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_t;

#define SYSTICK_BASE  (0xE000E010UL)
#define SYSTICK       ((SysTick_t *) SYSTICK_BASE)

// -----------------------------------------------------------------------------

static volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void)
{
    ++ms_ticks;
}

#define SYSCLK_HZ  16000000UL
#define SYSTICK_RELOAD_1MS ((SYSCLK_HZ / 1000U) - 1U)

void systick_init(void)
{
    SYSTICK->LOAD = SYSTICK_RELOAD_1MS;
    SYSTICK->VAL = 0;
    SYSTICK->CTRL = (1U << 0) | (1U << 1) | (1U << 2);
}

// -----------------------------------------------------------------------------
// NVIC
// -----------------------------------------------------------------------------
typedef struct {
    volatile uint32_t ISER;       // Interrupt Set-Enable Register
    volatile uint32_t _res1[31];  // Reserved
    volatile uint32_t ICER;       // Interrupt Clear Enable Register
    volatile uint32_t _res2[31];  // Reserved
    volatile uint32_t ISPR;       // Interrupt Set-Pending Register
    volatile uint32_t _res3[31];  // Reserved
    volatile uint32_t ICPR;       // Interrupt Clear-Pending Register
    volatile uint32_t _res4[31];  // Reserved
    volatile uint32_t IPR[8];     // Interrupt Priority Registers
} NVIC_t;

#define NVIC_BASE  (0xE000E100UL)
#define NVIC       ((NVIC_t *) NVIC_BASE)

// -----------------------------------------------------------------------------
// USART
// -----------------------------------------------------------------------------
typedef struct {
  volatile uint32_t CR1;    // 0x00  Control Register 1
  volatile uint32_t CR2;    // 0x04  Control Register 2
  volatile uint32_t CR3;    // 0x08  Control Register 3
  volatile uint32_t BRR;    // 0x0C  Baud Rate Register
  volatile uint32_t GTPR;   // 0x10  Guard Time and Prescaler Register
  volatile uint32_t RTOR;   // 0x14  Receiver Timeout Register
  volatile uint32_t RQR;    // 0x18  Request Register
  volatile uint32_t ISR;    // 0x1C  Interrupt and Status Register
  volatile uint32_t ICR;    // 0x20  Interrupt Flag Clear Register
  volatile uint32_t RDR;    // 0x24  Receive Data Register
  volatile uint32_t TDR;    // 0x28  Transmit Data Register
  volatile uint32_t PRESC;  // 0x2C  Prescaler Register
} USART_t;

#define USART2_BASE  0x40004400UL
#define USART2       ((USART_t *) USART2_BASE)

// -----------------------------------------------------------------------------

#define USART2_BRR_115200  139;
#define NEWLINE            "\r\n"

void usart2_init(void)
{
    RCC_APBENR1 |= (1U << 17);
    RCC_IOPENR  |= (1U << 0);

    // PA2 -> AF1 (USART2_TX) via AFRL[11:8]
    GPIOA->MODER &= ~(3U << 4);
    GPIOA->MODER |=  (2U << 4);
    GPIOA->AFRL  &= ~(0xFU << 8);
    GPIOA->AFRL  |=  (0x1U << 8);

    // PA3 -> AF1 (USART2_RX) via AFRL[15:12]
    GPIOA->MODER &= ~(3U << 6);
    GPIOA->MODER |=  (2U << 6);
    GPIOA->AFRL  &= ~(0xFU << 12);
    GPIOA->AFRL  |=  (0x1U << 12);

    USART2->BRR = USART2_BRR_115200;

    USART2->CR1 |= (1U << 3);  // TE
    USART2->CR1 |= (1U << 2);  // RE
    USART2->CR1 |= (1U << 5);  // RXNEIE - Enable RX Interrupt
    USART2->CR1 |= (1U << 0);  // UE

    // Enable USART2 IRQ (IRQ28) in NVIC
    NVIC->ISER |= (1U << 28);
}

void usart2_send_byte(uint8_t byte)
{
    while (!(USART2->ISR & (1U << 7)));  // wait TXE
    USART2->TDR = byte;
}

void usart2_send_string(const char *str)
{
    while (*str) {
        usart2_send_byte(*str);
        ++str;
    }
    while (!(USART2->ISR & (1U << 6)));  // wait TC
}

void usart2_send_uint_hex(uint32_t value)
{
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t nibble = value >> (32 - 4*(i+1)) & 0xF;
        if (nibble >= 10) {
            usart2_send_byte(nibble-10 + 'A');
        } else {
            usart2_send_byte(nibble + '0');
        }
    }
    while (!(USART2->ISR & (1U << 6)));
}

void USART2_IRQHandler(void)
{
    // Firstly, clear the Overrun, otherwise RXNE may not fire correctly
    if (USART2->ISR & (1U << 3)) {
        USART2->ICR |= (1U << 3);
    }

    // Byte Received
    if (USART2->ISR & (1U << 5)) {
        if (!rb_insert(USART2->RDR)) {
            usart2_send_string("Ring Buffer is Full. Program is stopped."NEWLINE);
            while (1);
        }
    }
}

// -----------------------------------------------------------------------------

#define BLINK_STEP_SIZE    0x7FU
#define BLINK_SPEED_LIMIT  0x7F0U

typedef enum {
    LED_BLINK_STATE,
    LED_MANUAL_STATE,
} LED_STATE;

// -----------------------------------------------------------------------------

// minicom -b 115200 -D /dev/ttyACM0
int main(void)
{
    systick_init();
    usart2_init();

    // GPIOA PA5 LED initialization
    RCC_IOPENR    |=  (1U <<  0);
    GPIOA->MODER  &= ~(3U << 10);
    GPIOA->MODER  |=  (1U << 10);
    GPIOA->OTYPER &= ~(1U <<  5);
    GPIOA->PUPDR  &= ~(3U << 10);
    GPIOA->BSRR    =  (1U << 21);  // LED off

    uint8_t is_led_on = 0;

    uint32_t blink_interval = BLINK_STEP_SIZE;
    uint32_t last = 0;

    LED_STATE led_state = LED_BLINK_STATE;

    usart2_send_string("UART IRQ-driven RX ready"NEWLINE);
    usart2_send_string("b=blink f=faster s=slower o=on x=off ?=help"NEWLINE);
    usart2_send_string("> ");

    while (1) {
        uint32_t now = ms_ticks;

        uint8_t cmd;
        while (rb_fetch(&cmd)) {
            usart2_send_byte(cmd);
            usart2_send_string(NEWLINE);

            switch (cmd) {
                case 'B':
                case 'b': {
                    led_state = LED_BLINK_STATE;
                    usart2_send_string("LED: Blink"NEWLINE);
                } break;

                case 'F':
                case 'f': {
                    blink_interval -= (blink_interval <= BLINK_STEP_SIZE) ? 0 : BLINK_STEP_SIZE;
                    usart2_send_string("Blink Speed: ");
                    usart2_send_uint_hex(blink_interval);
                    usart2_send_string(NEWLINE);
                } break;

                case 'S':
                case 's': {
                    blink_interval += (blink_interval >= BLINK_SPEED_LIMIT) ? 0 : BLINK_STEP_SIZE;
                    usart2_send_string("Blink Speed: ");
                    usart2_send_uint_hex(blink_interval);
                    usart2_send_string(NEWLINE);
                } break;

                case 'O':
                case 'o': {
                    led_state = LED_MANUAL_STATE;
                    is_led_on = 1;
                    GPIOA->BSRR = (1U << 5);
                    usart2_send_string("LED: On"NEWLINE);
                } break;

                case 'X':
                case 'x': {
                    led_state = LED_MANUAL_STATE;
                    is_led_on = 0;
                    GPIOA->BSRR = (1U << 21);
                    usart2_send_string("LED: Off"NEWLINE);
                } break;

                case '?': {
                    usart2_send_string("b=blink f=faster s=slower o=on x=off"NEWLINE);
                } break;

                default: {
                    usart2_send_string("Unknown Command. Press `?` for help"NEWLINE);
                }
            }

            usart2_send_string("> ");
        }

        if (led_state == LED_BLINK_STATE) {
            if ((now - last) >= blink_interval) {
                last = now;
                is_led_on = !is_led_on;
                GPIOA->BSRR = (is_led_on) ? (1U << 5) : (1U << 21);
            }
        }

        __asm volatile ("wfi");
    }

    return 0;
}
