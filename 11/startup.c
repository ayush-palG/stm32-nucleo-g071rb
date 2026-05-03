#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sidata;         // where .data is stored in Flash
extern uint32_t _sdata, _edata;  // .data section boundaries
extern uint32_t _sbss, _ebss;

void Reset_Handler(void);
int main(void);

void NMI_Handler                 (void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler           (void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler                 (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler              (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler             (void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector"))) uint32_t vectors[] = {
    (uint32_t) &_estack,
    (uint32_t) &Reset_Handler,
    (uint32_t) &NMI_Handler,
    (uint32_t) &HardFault_Handler,
    0, 0, 0, 0, 0, 0, 0,
    (uint32_t) &SVC_Handler,
    0, 0, 
    (uint32_t) &PendSV_Handler,
    (uint32_t) &SysTick_Handler
};

void Default_Handler(void)
{
    while (1);
}

void Reset_Handler(void)
{
    // 1. Copy .data from Flash to SRAM
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
	*dst++ = *src++;
    }

    // 2. Zero out .bss in SRAM
    dst = &_sbss;
    while (dst < &_ebss) {
	*dst++ = 0;
    }
  
    main();

    // In case we got out of main function, should never reach here, trap if it does
    while (1);
}
