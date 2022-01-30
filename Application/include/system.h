#ifndef SYSTEM_H
#define SYSTEM_H

/* Interrupt priorities
 * Remark: Lower equals higher priority
 *
 * See: https://wiki.segger.com/Interrupt_prioritization
 */
#define CMSIS_IRQn_OFFSET       (0u)
#define EMBOS_IRQn_OFFSET       (16u)
#define IRQn_OFFSET             (CMSIS_IRQn_OFFSET)

#define EMBOS_IRQ_PRIO          (4u)
#define ZERO_LATENCY_IRQ_PRIO   (0u)

#define RTC_IRQ_PRIO            (2u + EMBOS_IRQ_PRIO)
#define SPI0_IRQ_PRIO           (3u + EMBOS_IRQ_PRIO)
#define XDMAC_IRQ_PRIO          (0u + EMBOS_IRQ_PRIO)
#define PIOD_IRQ_PRIO           (1u + EMBOS_IRQ_PRIO)
#define TWIHS0_IRQ_PRIO         (3u + EMBOS_IRQ_PRIO)


void sys_vInit(void);

#endif /* SYSTEM_H */
