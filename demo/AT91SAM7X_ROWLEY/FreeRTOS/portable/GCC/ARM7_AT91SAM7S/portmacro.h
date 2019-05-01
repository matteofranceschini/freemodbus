/*
 * MODBUS Library: SAM7X Port
 * Copyright (c) 2007 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * $Id$
 */
/*
 * FreeRTOS.org V4.4.0 - Copyright (C) 2003-2007 Richard Barry.
 *
 * This file is part of the FreeRTOS.org distribution.
 *
 * FreeRTOS.org is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FreeRTOS.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeRTOS.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * A special exception to the GPL can be applied should you wish to distribute
 * a combined work that includes FreeRTOS.org, without being obliged to provide
 * the source code for any proprietary components.  See the licensing section
 * of http://www.FreeRTOS.org for full details of how and when the exception
 * can be applied.
 *
 ****************************************************************************
 * See http://www.FreeRTOS.org for documentation, latest information, license
 * and contact details.  Please ensure to read the configuration and relevant
 * port sections of the online documentation.
 *
 * Also see http://www.SafeRTOS.com for an IEC 61508 compliant version along
 * with commercial development and support options.
 ***************************************************************************
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

/* ----------------------- Defines (Type definitions) -----------------------*/
#define portCHAR char
#define portFLOAT float
#define portDOUBLE double
#define portLONG long
#define portSHORT short
#define portSTACK_TYPE unsigned portLONG
#define portBASE_TYPE portLONG

#if (configUSE_16_BIT_TICKS == 1)
typedef unsigned portSHORT portTickType;
#define portMAX_DELAY (portTickType)0xffff
#else
typedef unsigned portLONG portTickType;
#define portMAX_DELAY (portTickType)0xffffffff
#endif

/* ----------------------- Defines (Architecture specific) ------------------*/

#define portSTACK_GROWTH (-1)
#define portTICK_RATE_MS ((portTickType)1000 / configTICK_RATE_HZ)
#define portBYTE_ALIGNMENT 4
#define portNOP() asm volatile("NOP");

/*
 * portRESTORE_CONTEXT, portRESTORE_CONTEXT, portENTER_SWITCHING_ISR
 * and portEXIT_SWITCHING_ISR can only be called from ARM mode, but
 * are included here for efficiency.  An attempt to call one from
 * THUMB mode code will result in a compile time error.
 */
#define portRESTORE_CONTEXT()                                                                                                   \
    do                                                                                                                          \
    {                                                                                                                           \
        extern volatile void *volatile pxCurrentTCB;                                                                            \
        extern volatile unsigned portLONG ulCriticalNesting;                                                                    \
                                                                                                                                \
        /* Set the LR to the task stack. */                                                                                     \
        asm volatile(                                                                                                           \
            "LDR    R0, =pxCurrentTCB\n\t"                                                                                      \
            "LDR    R0, [R0]\n\t"                                                                                               \
            "LDR    LR, [R0]\n\t"                                                                                               \
                                                                                                                                \
            /* The critical nesting depth is the first item on the stack. */ /* Load it into the ulCriticalNesting variable. */ \
            "LDR    R0, =ulCriticalNesting\n\t"                                                                                 \
            "LDMFD  LR!, {R1}\n\t"                                                                                              \
            "STR    R1, [R0]\n\t"                                                                                               \
                                                                                                                                \
            /* Get the SPSR from the stack. */                                                                                  \
            "LDMFD  LR!, {R0}\n\t"                                                                                              \
            "MSR    SPSR, R0\n\t"                                                                                               \
                                                                                                                                \
            /* Restore all system mode registers for the task. */                                                               \
            "LDMFD  LR, {R0-R14}^\n\t"                                                                                          \
            "NOP\n\t"                                                                                                           \
                                                                                                                                \
            /* Restore the return address. */                                                                                   \
            "LDR    LR, [LR, #+60]\n\t"                                                                                         \
                                                                                                                                \
            /* And return - correcting the offset in the LR to obtain the */ /* correct address. */                             \
            "SUBS   PC, LR, #4\n\t");                                                                                           \
        (void)ulCriticalNesting;                                                                                                \
        (void)pxCurrentTCB;                                                                                                     \
    } while (0)

#define portSAVE_CONTEXT()                                                \
    do                                                                    \
    {                                                                     \
        extern volatile void *volatile pxCurrentTCB;                      \
        extern volatile unsigned portLONG ulCriticalNesting;              \
                                                                          \
        /* Push R0 as we are going to use the register. */                \
        asm volatile(                                                     \
            "STMDB  SP!, {R0}\n\t"                                        \
                                                                          \
            /* Set R0 to point to the task stack pointer. */              \
            "STMDB	SP,{SP}^\n\t"                                          \
            "NOP\n\t"                                                     \
            "SUB    SP, SP, #4\n\t"                                       \
            "LDMIA	SP!,{R0}\n\t"                                          \
                                                                          \
            /* Push the return address onto the stack. */                 \
            "STMDB	R0!, {LR}\n\t"                                         \
                                                                          \
            /* Now we have saved LR we can use it instead of R0. */       \
            "MOV    LR, R0\n\t"                                           \
                                                                          \
            /* Pop R0 so we can save it onto the system mode stack. */    \
            "LDMIA	SP!, {R0}\n\t"                                         \
                                                                          \
            /* Push all the system mode registers onto the task stack. */ \
            "STMDB	LR,{R0-LR}^\n\t"                                       \
            "NOP\n\t"                                                     \
            "SUB    LR, LR, #60\n\t"                                      \
                                                                          \
            /* Push the SPSR onto the task stack. */                      \
            "MRS    R0, SPSR\n\t"                                         \
            "STMDB	LR!, {R0}\n\t"                                         \
                                                                          \
            "LDR    R0, =ulCriticalNesting\n\t"                           \
            "LDR    R0, [R0]\n\t"                                         \
            "STMDB	LR!, {R0}\n\t"                                         \
                                                                          \
            /* Store the new top of stack for the task. */                \
            "LDR    R0, =pxCurrentTCB\n\t"                                \
            "LDR    R0, [R0]\n\t"                                         \
            "STR    LR, [R0]\n\t");                                       \
        (void)ulCriticalNesting;                                          \
        (void)pxCurrentTCB;                                               \
    } while (0)

#define portENTER_SWITCHING_ISR()                                            \
    do                                                                       \
    {                                                                        \
        /* Save the context of the interrupted task. */                      \
        portSAVE_CONTEXT();                                                  \
                                                                             \
        /* We don't know the stack requirements for the ISR, so the frame */ \
        /* pointer will be set to the top of the task stack, and the stack*/ \
        /* pointer left where it is.  The IRQ stack will get used for any */ \
        /* functions calls made by this ISR. */                              \
        asm volatile("SUB		R11, LR, #4");                               \
        {

#define portEXIT_SWITCHING_ISR(SwitchRequired)                      \
    /* If a switch is required then we just need to call */         \
    /* vTaskSwitchContext() as the context has already been */      \
    /* saved. */                                                    \
    if (SwitchRequired)                                             \
    {                                                               \
        vTaskSwitchContext();                                       \
    }                                                               \
    }                                                               \
    /* Restore the context of which ever task is now the highest */ \
    /* priority that is ready to run. */                            \
    portRESTORE_CONTEXT();                                          \
    }                                                               \
    while (0)

#define portYIELD() asm volatile("SWI");

/*
 * The interrupt management utilities can only be called from ARM mode.  When
 * THUMB_INTERWORK is defined the utilities are defined as functions in
 * portISR.c to ensure a switch to ARM mode.  When THUMB_INTERWORK is not
 * defined then the utilities are defined as macros here - as per other ports.
 */
#ifdef THUMB_INTERWORK

extern void vPortDisableInterruptsFromThumb(void) __attribute__((naked));
extern void vPortEnableInterruptsFromThumb(void) __attribute__((naked));

#define portDISABLE_INTERRUPTS() vPortDisableInterruptsFromThumb()
#define portENABLE_INTERRUPTS() vPortEnableInterruptsFromThumb()

#else

#define portDISABLE_INTERRUPTS()                                        \
    do                                                                  \
    {                                                                   \
        asm volatile(                                                   \
            "STMDB  SP!, {R0}\n\t"     /* Push R0.	*/                   \
            "MRS    R0, CPSR\n\t"      /* Get CPSR. */                  \
            "ORR    R0, R0, #0xC0\n\t" /* Disable IRQ, FIQ. */          \
            "MSR    CPSR, R0\n\t"      /* Write back modified value.	*/ \
            "LDMIA  SP!, {R0}");       /* Pop R0. */                    \
    } while (0)

#define portENABLE_INTERRUPTS()                                            \
    do                                                                     \
    {                                                                      \
        asm volatile(                                                      \
            "STMDB	SP!, {R0}\n\t"        /* Push R0.	*/                   \
            "MRS    R0, CPSR\n\t"         /* Get CPSR. */                  \
            "BIC    R0, R0, #0xC0	\n\t" /* Enable IRQ, FIQ. */           \
            "MSR    CPSR, R0\n\t"         /* Write back modified value.	*/ \
            "LDMIA	SP!, {R0}");          /* Pop R0. */                    \
    } while (0)
#endif

extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

#define portENTER_CRITICAL() vPortEnterCritical();
#define portEXIT_CRITICAL() vPortExitCritical();

/* ----------------------- Defines (Other) ----------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void *pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters) void vFunction(void *pvParameters)

#endif
