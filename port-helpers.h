#ifndef _PORT_HELPERS_H_
#define _PORT_HELPERS_H_

#include <msp430.h>

#define BIT_set(container, bits) (container) |= (bits)
#define BIT_unset(container, bits) (container) &= ~(bits)
#define BIT_toggle(container, bits) (container) ^= (bits)

#define PIN_setModePrimPeriph(port, pins) \
{\
    BIT_set(port##SEL, (pins)); \
    BIT_unset(port##SEL2, (pins)); \
}

#define PIN_setModeSecPeriph(port, pins) \
{\
    BIT_set(port##SEL, (pins)); \
    BIT_set(port##SEL2, (pins)); \
}

#define PIN_setModeGPO(port, pins) \
{\
    BIT_unset(port##SEL, (pins)); \
    BIT_unset(port##SEL2, (pins)); \
    BIT_set(port##DIR, (pins)); \
}

#define PIN_outHigh(port, pins) BIT_set(port##OUT, (pins))
#define PIN_outLow(port, pins) BIT_unset(port##OUT, (pins))

#define PIN_setModeGPI_nopull(port, pins) \
{\
    BIT_unset(port##SEL, (pins)); \
    BIT_unset(port##SEL2, (pins)); \
    BIT_unset(port##DIR, (pins)); \
    BIT_unset(port##REN, (pins)); \
}

#define PIN_setModeGPI_pullup(port, pins) \
{\
    PIN_setModeGPI_nopull(port, (pins)); \
    BIT_set(port##OUT, (pins)); \
    BIT_set(port##REN, (pins)); \
}

#define PIN_setModeGPI_pulldown(port, pins) \
{\
    PIN_setModeGPI_nopull(port, pins); \
    BIT_unset(port##OUT, (pins)); \
    BIT_set(port##REN, (pins)); \
}

#define PIN_readIn(port, pins) (port##IN & pins)

#endif /* include guard */
