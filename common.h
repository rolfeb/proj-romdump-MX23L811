#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdarg.h>

extern void     serial_init(void);
extern void     serial_write(char *fmt, ...);
extern void     serial_write_string(char *string);


#endif /* __COMMON_H__ */
