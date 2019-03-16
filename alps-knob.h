#ifndef _ALPS-KNOB_H_
#define _ALPS-KNOB_H_

#include <stdint.h>
#include <stdbool.h>

#define KNOB_INC_NORMAL 1
#define KNOB_INC_PRESSED 3

void knob_setup(void);
int8_t knob_get_changes(void);
bool knob_changed(void);

#endif /* include guard */
