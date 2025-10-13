#ifndef BEEPER_DRIVER_H
#define BEEPER_DRIVER_H
#include <STD/TYPEDEF.h>

/**
 * @brief Sets the frequency for the PC speaker by programming PIT Channel 2.
 * * @param f The desired frequency in Hz. Minimum supported is ~18 Hz.
 */
VOID SET_BEEPER_FREQUENCY(U32 f);
VOID START_BEEPER();
VOID STOP_BEEPER();
#endif // BEEPER_DRIVER_H