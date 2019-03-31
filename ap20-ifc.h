#ifndef _AP20-IFC_H_
#define _AP20-IFC_H_

#include <stdint.h>
#include <stdbool.h>

void ap20_init(void);
void ap20_change_level(int8_t);

void ap20_process_bytes(void);
uint8_t ap20_current_level(void);

#endif /* include guard */
