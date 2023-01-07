/** @file */

#include <stdio.h>
#include <stdarg.h>
#include "RTOS.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "trace.h"


/* Buffer index 0 is for terminal and
 * index 1 is for logging.
 */
#define SEGGER_RTT_BUFFER_INDEX_TERM  (0)
#define SEGGER_RTT_BUFFER_INDEX_FILE  (2)


extern OS_MUTEX          traceMutex;


/**
 * Target printf to SEGGER RTT virtual terminal.
 *
 * @param   term    Target terminal.
 *
 * @param   format  String format  followed by the
 *                  arguments for conversion.
 *
 * @retval  r       Bytes written.
 *
 * @note    Not to be called directly!
 */
int trace_terminal(int term, const char *format, ...)
{
    int     r;
    va_list args;

    /* Guard the virtual terminal else they can get mixed. */
    OS_MUTEX_LockBlocked(&traceMutex);

    SEGGER_RTT_SetTerminal(term);

    /* printf routed to SEGGER RTT */
    va_start (args, format);
    r = vprintf(format, args);
    va_end(args);
    
    OS_MUTEX_Unlock(&traceMutex);

    return r;
}


/**
 * Target printf to SEGGER RTT file logging.
 *
 * @param   format  String format  followed by the
 *                  arguments for conversion.
 *
 * @retval  r       Bytes written.
 *
 * @note    Not to be called directly!
 */
int trace_file(const char *format, ...)
{
    int     r;
    va_list args;
    char    buf[SEGGER_RTT_PRINTF_BUFFER_SIZE];

    va_start (args, format);
    r = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    r = SEGGER_RTT_WriteString(SEGGER_RTT_BUFFER_INDEX_FILE, buf);

    return r;
}
