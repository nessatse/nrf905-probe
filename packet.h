#ifndef _PACKET_H_
#define _PACKET_H_
#include <stdint.h>

/* Packet format

<---1 byte --->
+-------------+
|  Length     |
+-------------+
|  Unit ID    |
+-------------+
| Packet ID   |
+-------------+
| Sensor Type |
+-------------+
| Sensor ID   |
+-------------+
| Value ...   |
+-------------+
| Sensor Type |
    ...

*/

struct _pkt_hdr {
    uint8_t pktlen;
    uint8_t unitid;
    uint8_t pktid;
    uint8_t data[];
} __attribute__((packed));
typedef struct _pkt_hdr pkt_hdr_t;

struct _pkt_tv {
    uint8_t id;
    uint8_t value[];
} __attribute__((packed));
typedef struct _pkt_tv pkt_tv_t;
// LM75 Temp Sensor
struct _tv_lm75 {
    uint8_t id;
    uint16_t value;
} __attribute__((packed));
typedef struct _tv_lm75 tv_lm75_t;
//Battery Status (1-good,0-bad);
struct _tv_bat {
    uint8_t id;
    uint8_t value;
} __attribute__((packed));
typedef struct _tv_bat tv_bat_t;

// Sensor types

#define SENS_LM75    1
#define SENS_BAT     2
#endif // _PACKET_H_
