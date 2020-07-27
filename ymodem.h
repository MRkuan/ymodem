//*****************************************************************************
// file    : ymodem.h
// It's a ymodem.h file
//
// Copyright (c) 2011-2020  co. Ltd. All rights reserved
//
// Change Logs:
// Date               Author              Note
// 2020/07/09         kuan                First draft version
//
//*****************************************************************************

#ifndef __YMODEM_H__
#define __YMODEM_H__

//*****************************************************************************
//
//! \addtogroup Ymodem
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include "Platform_Types.h"

#define YMODEM_TIME_BASE                       (10) /** uint 10ms */
#define YMODEM_TIME_MS(cnt)                    ((uint32)cnt/YMODEM_TIME_BASE)
#define YMODEM_HAND_SHAKE_TIME_MS              YMODEM_TIME_MS(10000)
#define YMODEM_TRANS_TIME_MS                   YMODEM_TIME_MS(3000)
#define YMODEM_TRANS_FINISH_TIME_MS            YMODEM_TIME_MS(3000)


#define PACKET_HEADER_INDEX         (0)
#define PACKET_SEQNO_INDEX          (1)
#define PACKET_SEQNO_COMP_INDEX     (2)
#define PACKET_DATA_INDEX           (3)

#define PACKET_HEADER               (3)     /* start, block, block-complement */
#define PACKET_TRAILER              (2)     /* CRC bytes */
#define PACKET_OVERHEAD             (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE                 (128)
#define PACKET_1K_SIZE              (1024)

#define PACKET_SOH_PKG_SIZE (PACKET_SIZE + PACKET_OVERHEAD)
#define PACKET_STX_PKG_SIZE (PACKET_1K_SIZE + PACKET_OVERHEAD)

#define NACK_RETRY                  (3)
#define END_SESSION_SEND_CAN_NUM    (7)

typedef enum
{
    YMODEM_CODE_NONE = 0x00,      /*< no code */
    YMODEM_CODE_SOH  = 0x01,      /*< start of 128-byte data packet */
    YMODEM_CODE_STX  = 0x02,      /*< start of 1024-byte data packet */
    YMODEM_CODE_EOT  = 0x04,      /*< end of transmission */
    YMODEM_CODE_ACK  = 0x06,      /*< acknowledge */
    YMODEM_CODE_NAK  = 0x15,      /*< negative acknowledge */
    YMODEM_CODE_CAN  = 0x18,      /*< two of these in succession aborts transfer */
    YMODEM_CODE_C    = 0x43,      /*< 'C' == 0x43, request 16-bit CRC */
} ymodem_code_t;

typedef enum
{
    YMODEM_STATE_IDLE,            /*< idle state */
    YMODEM_STATE_ESTABLISHING,    /*< set when C is send */
    YMODEM_STATE_ESTABLISHED,     /*< set when we've got the packet 0 and sent ACK and second C */
    YMODEM_STATE_TRANSMITTING,    /*< set when the sender respond to our second C and recviever got a real * data packet. */
    YMODEM_STATE_FINISHED,        /*< set when transmission is really finished, i.e., after the NAK, C, final NULL packet stuff. */
} ymodem_state_t;

typedef enum
{
    YMODEM_RESULT_SUCESS,

    YMODEM_RESULT_HANDSHAKE_TIMEOUT,
    YMODEM_RESULT_TRANS_TIMEOUT,
    YMODEM_RESULT_FINISH_TIMEOUT,
    YMODEM_RESULT_ERROR_CODE,
    YMODEM_RESULT_NAK_RETRY_OUT,
    YMODEM_RESULT_PASSIVE_CANCEL,
    YMODEM_RESULT_INITIATIVE_CANCEL,
    YMODEM_RESULT_ERROR_SEQ,
} ymodem_result_t;

typedef struct
{
    boolean (*put_data)(uint8 *data,uint32 len);       /**< put data */
    boolean (*get_data)(uint8 *data,uint32 len);       /**< put data */
} ymodem_port_t;

typedef struct
{
    uint8 packet_data[PACKET_STX_PKG_SIZE]; /**< support 1K include 128 */
    uint32 packet_data_cnt;                 /**< count about packet data cnt */
    uint32 packet_data_size;                /**< count about packet data size */
    ymodem_state_t state;                   /**< ymodem state machine state */

    uint32 recv_correct_cnt;                /**< when receive correct data ,this cnt +1 */
    uint32 recv_error_cnt;                  /**< when receive error data ,this cnt +1 */

    uint32 time_cnt;
    uint32 nack_retry_cnt;
} ymodem_data_t;

typedef ymodem_code_t (*ymodem_cb_t)(uint8 *buf, uint32 len);
typedef void (*ymodem_result)(ymodem_result_t result);

typedef struct
{
    ymodem_cb_t   on_begin;
    ymodem_cb_t   on_data;
    ymodem_cb_t   on_end;
    ymodem_result on_result;
} ymodem_cbs_t;

typedef struct
{
    ymodem_data_t  data;
    ymodem_port_t  port;
    ymodem_cbs_t   cbs;
} ymodem_t;

extern const char *ymodem_result_info[];

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
boolean ymodem_init(ymodem_t *this);
void ymodem_mainfunction(ymodem_t *this);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

//*****************************************************************************
//
// Close the Doxygen group.
//! @{
//
//*****************************************************************************
#endif //  __YMODEM_H__ 
