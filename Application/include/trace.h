/** @file */

#ifndef TRACE_H
#define TRACE_H

#define TRACE_INFO(...)               trace_terminal(0, __VA_ARGS__)
#define TRACE_ERROR(...)              trace_terminal(1, __VA_ARGS__)
#define TRACE_LOG(...)                trace_terminal(2, __VA_ARGS__)
#define TRACE_FILE(...)               trace_file(__VA_ARGS__)


int trace_terminal(int term, const char *format, ...);
int trace_file(const char *format, ...);

#endif /* TRACE_H */
