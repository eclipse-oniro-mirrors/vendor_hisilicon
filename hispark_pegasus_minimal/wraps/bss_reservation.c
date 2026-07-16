/*
 * BSS reservation for low-memory simulation (Hi3861).
 *
 * Reserves BSS space, which grows into RAM and reduces the available heap,
 * simulating tight RAM conditions without changing the actual RAM size.
 *
 * Usage: --gn-args bss_reservation_size=<bytes>
 *   0     = no reservation (default, normal operation)
 *   >0    = reserve <bytes> of BSS, reducing heap by the same amount
 *
 * Equivalent to the Hi3863 (WS63) bss_reservation_size mechanism.
 *
 * The array is referenced by BssReservationInfo (CORE_INIT) to prevent the
 * linker from stripping it as an unreferenced symbol (--gc-sections).
 */
#include <stdint.h>
#include <stdio.h>
#include <ohos_init.h>

#ifndef CONFIG_BSS_RESERVATION_SIZE
#define CONFIG_BSS_RESERVATION_SIZE 0
#endif

#if CONFIG_BSS_RESERVATION_SIZE > 0
uint8_t g_bssReservation[CONFIG_BSS_RESERVATION_SIZE] __attribute__((used));

/* Print reservation info at boot; also references g_bssReservation so the
 * linker keeps it. Registered via CORE_INIT (runs early in OHOS_SystemInit). */
static void BssReservationInfo(void)
{
    printf("[BssReservation] size:%u ptr:%p\r\n",
           (unsigned int)CONFIG_BSS_RESERVATION_SIZE, (void *)g_bssReservation);
}
CORE_INIT(BssReservationInfo);
#endif
