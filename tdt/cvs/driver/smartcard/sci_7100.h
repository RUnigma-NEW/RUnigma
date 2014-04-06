#ifndef _SCI_7100_H
#define _SCI_7100_H

#define SYS_CFG_BASE_ADDRESS     	0x19001000
#define SYS_CFG7                  	0x11C

/******* SC 0 *******/

#define SCI0_INT_DETECT             80
#define SCI0_INT_RX_TX              123

#define SCI0_BASE_ADDRESS           0x18048000

#define ASC0_BASE_ADDRESS           0x18030000 
#define ASC0_BAUDRATE               0x00
#define ASC0_TX_BUF                 0x004
#define ASC0_RX_BUF                 0x008
#define ASC0_CTRL                   0x00C
#define ASC0_INT_EN                 0x010
#define ASC0_STA                    0x014
#define ASC0_GUARDTIME              0x018
#define ASC0_TIMEOUT                0x01C
#define ASC0_TX_RST                 0x020
#define ASC0_RX_RST                 0x024
#define ASC0_RETRIES                0x028

/******* SC 1 *******/

#define SCI1_INT_DETECT             84
#define SCI1_INT_RX_TX              122

#define SCI1_BASE_ADDRESS           0x18049000

#define ASC1_BASE_ADDRESS           0x18031000
#define ASC1_BAUDRATE               0x00
#define ASC1_TX_BUF                 0x004
#define ASC1_RX_BUF                 0x008
#define ASC1_CTRL                   0x00C
#define ASC1_INT_EN                 0x010
#define ASC1_STA                    0x014
#define ASC1_GUARDTIME              0x018
#define ASC1_TIMEOUT                0x01C
#define ASC1_TX_RST                 0x020
#define ASC1_RX_RST                 0x024
#define ASC1_RETRIES                0x028

/******* Board-specific defines *******/

#define ACTIVE_HIGH                 1
#define ACTIVE_LOW                  0
#define SCI_CLASS                   1 //SCI_CLASS_A     /**< Operating class of SCI */

#endif  /* _SCI_7100_H */
