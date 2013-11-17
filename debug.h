#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG
#define DBG_INIT() uart_init()
#define DBG_PUTCH(c) uart_tx((c))
#define DBG_STR(s) uart_str((s))
#define DBG_PUTHEX(c) uart_tx_hex((c))
#define DBG_TXDONE() uart_completetx()
#else
#define DBG_INIT()
#define DBG_PUTCH(c)
#define DBG_STR(s)
#define DBG_PUTHEX(c)
#define DBG_TXDONE()
#endif //DEBUG

#endif // _DEBUG_H_
