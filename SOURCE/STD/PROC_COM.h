/**
 * PROC_COM.h
 *
 * Process communication utilities for programs and processes.
 *
 * NOT usable in kernel code.
 * 
 * Search for PROC_COM for process communication utilities.
 * Search for SHELLCOM for shell communication utilities.
 * Search for KRNLCOM for kernel communication utilities.
 */

#ifndef PROC_COM_H
#define PROC_COM_H

#include <STD/TYPEDEF.h>

/**
 * PROC_COM
 */
U32 PROC_GETPID(U0);

U32 PROC_GETPPID(U0); 

/**
 * KRNLCOM
 */

/**
 * SHELLCOM
 */
#endif //PROC_COM_H