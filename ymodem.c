//*****************************************************************************
// file    : ymodem.c(only support receive)
// It's a ymodem.c file,ref: rt-thread
//
// Copyright (c) 2011-2020  co. Ltd. All rights reserved
//
// Change Logs:
// Date               Author              Note
// 2020/07/09         kuan                First draft version(only support receive)
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup Ymodem
//! @{
//
//*****************************************************************************
#include "ymodem.h"

#define  YMODEM_GET_HEADER()             (this->data.packet_data[PACKET_HEADER_INDEX])
#define  YMODEM_GET_SEQNO()              (this->data.packet_data[PACKET_SEQNO_INDEX])
#define  YMODEM_GET_SEQNO_COMP()         (this->data.packet_data[PACKET_SEQNO_COMP_INDEX])
#define  YMODEM_GET_DATA_PTR()           (&(this->data.packet_data[PACKET_DATA_INDEX]))
#define  YMODEM_GET_DATA_SIZE()          ((YMODEM_CODE_SOH == YMODEM_GET_HEADER()) ? PACKET_SIZE :PACKET_1K_SIZE)
#define  YMODEM_GET_CRC_1()              (this->data.packet_data[PACKET_DATA_INDEX+YMODEM_GET_DATA_SIZE()])
#define  YMODEM_GET_CRC_2()              (this->data.packet_data[PACKET_DATA_INDEX+YMODEM_GET_DATA_SIZE()+1])

const char *ymodem_result_info[] =
{
    "SUCESS",
    "HANDSHAKE_TIMEOUT",
    "TRANS_TIMEOUT",
    "FINISH_TIMEOUT",
    "ERROR_CODE",
    "NAK_RETRY_OUT",
    "PASSIVE_CANCEL",
    "INITIATIVE_CANCEL",
    "ERROR_SEQ",
};

static const uint16 ccitt_table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/**
  * \brief
  *
  * \param this
  *
  * \return
  */
static boolean ymodem_crc16_is_correct(ymodem_t *this)
{
    boolean ret = false;
    uint8 *q;
    uint32 len;
    uint16 crc = 0;
    uint16 recv_crc = 0;

    if (YMODEM_CODE_SOH == YMODEM_GET_HEADER() || YMODEM_CODE_STX == YMODEM_GET_HEADER())
    {
        q = YMODEM_GET_DATA_PTR();
        len = YMODEM_GET_DATA_SIZE();
        while (len-- > 0)
            crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];

        recv_crc = (YMODEM_GET_CRC_1() << 8 | YMODEM_GET_CRC_2());

        if (recv_crc == crc)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }

    return ret;
}


/**
  * \brief
  *
  * \param code
  *
  * \return
  */
static ymodem_set_reply_code(ymodem_t *this, ymodem_code_t code)
{
    uint8 __code = code;

    this->port.put_data(&__code, 1);
}

/**
  * \brief
  *
  * \param this
  * \param len
  *
  * \return
  */
static void ymodem_Set_result(ymodem_t *this, ymodem_result_t result)
{
    //18 18 18 18 18 08 08 08 08 08

    for (uint8 i = 0; i < END_SESSION_SEND_CAN_NUM; i++)
    {
        ymodem_set_reply_code(this, YMODEM_CODE_CAN);
    }

    this->cbs.on_result(result);
    this->data.state = YMODEM_STATE_IDLE;
}

/**
  * \brief
  *
  * \param this
  *
  * \return
  */
boolean ymodem_init(ymodem_t *this)
{
    this->data.recv_correct_cnt = 0;
    this->data.recv_error_cnt = 0;
    this->data.time_cnt = 0;
    this->data.nack_retry_cnt = 0;

    if (NULL == ymodem_set_reply_code || NULL == this->port.get_data)
    {
        return false;
    }

    if (NULL == this->cbs.on_begin || NULL == this->cbs.on_data
            || NULL == this->cbs.on_end || NULL == this->cbs.on_begin)
    {
        return false;
    }
    this->data.time_cnt = 0;
    this->data.packet_data_cnt = 0;
    this->data.state = YMODEM_STATE_ESTABLISHING;
    return true;
}

/**
  * \brief
  *
  * \param None
  *
  * \return
  */
static boolean ymodem_get_data(ymodem_t *this)
{
    uint8 data;
    boolean ret;

    while (1)
    {
        if (0 == this->data.packet_data_cnt)
        {
            ret = this->port.get_data(&data, 1);
            if (false == ret)
            {
                return false;
            }

            this->data.packet_data[this->data.packet_data_cnt++] = data;
            if (1 == this->data.packet_data_cnt)
            {
                switch (YMODEM_GET_HEADER())
                {
                case YMODEM_CODE_SOH:
                    this->data.packet_data_size = PACKET_SOH_PKG_SIZE;
                    break;
                case YMODEM_CODE_STX:
                    this->data.packet_data_size = PACKET_STX_PKG_SIZE;
                    break;
                case YMODEM_CODE_EOT:
                    this->data.packet_data_size = 1;
                    return true;
                default:
                    this->data.packet_data_size = 1;
                    ymodem_Set_result(this, YMODEM_RESULT_ERROR_CODE);
                    break;
                }
            }
        }

        if (0 != this->data.packet_data_cnt)
        {
            ret = this->port.get_data(&(YMODEM_GET_SEQNO()), this->data.packet_data_size - 1);
            if (true == ret)
            {
                if (0xFF != (YMODEM_GET_SEQNO() + YMODEM_GET_SEQNO_COMP()))
                {
                    ymodem_Set_result(this, YMODEM_RESULT_ERROR_SEQ);
                }

                if (true == ymodem_crc16_is_correct(this))
                {
                    return true;
                }
                else
                {
                    this->data.time_cnt = 0;
                    this->data.packet_data_cnt = 0;
                    ymodem_set_reply_code(this, YMODEM_CODE_NAK);
                    this->data.recv_error_cnt++;
                    if (this->data.nack_retry_cnt++ >= NACK_RETRY)
                    {
                        ymodem_Set_result(this, YMODEM_RESULT_NAK_RETRY_OUT);
                    }
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
}

/**
  * \brief
  *
  * \param this
  *
  * \return
  */
void ymodem_mainfunction(ymodem_t *this)
{
    switch (this->data.state)
    {
    case YMODEM_STATE_IDLE:
        break;

    case YMODEM_STATE_ESTABLISHING:
        if (0 == (this->data.time_cnt % YMODEM_TIME_MS(2000)))
        {
            ymodem_set_reply_code(this, YMODEM_CODE_C);
        }
        if (this->data.time_cnt++ >= YMODEM_HAND_SHAKE_TIME_MS)
        {
            ymodem_Set_result(this, YMODEM_RESULT_HANDSHAKE_TIMEOUT);
        }
        else
        {
            if (true == ymodem_get_data(this))
            {
                this->data.time_cnt = 0;
                this->data.packet_data_cnt = 0;
                if (YMODEM_CODE_SOH == YMODEM_GET_HEADER())
                {
                    if (this->cbs.on_begin)
                    {
                        ymodem_code_t code;
                        code = this->cbs.on_begin(YMODEM_GET_DATA_PTR(), YMODEM_GET_DATA_SIZE());
                        if (code != YMODEM_CODE_ACK)
                        {
                            ymodem_Set_result(this, YMODEM_RESULT_INITIATIVE_CANCEL);
                        }
                        else
                        {
                            ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                            this->data.recv_correct_cnt++;
                            this->data.state = YMODEM_STATE_ESTABLISHED;
                        }
                    }
                    else
                    {
                        ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                        this->data.recv_correct_cnt++;
                    }
                }
                else
                {
                    ymodem_Set_result(this, YMODEM_RESULT_ERROR_CODE);
                }
            }
        }
        break;

    case YMODEM_STATE_ESTABLISHED:
        ymodem_set_reply_code(this, YMODEM_CODE_C);
        this->data.state = YMODEM_STATE_TRANSMITTING;
        break;

    case YMODEM_STATE_TRANSMITTING:
        if (this->data.time_cnt++ >= YMODEM_TRANS_TIME_MS)
        {
            ymodem_Set_result(this, YMODEM_RESULT_TRANS_TIMEOUT);
        }
        else
        {
            if (true == ymodem_get_data(this))
            {
                this->data.time_cnt = 0;
                this->data.packet_data_cnt = 0;
                if (YMODEM_CODE_CAN == YMODEM_GET_HEADER())
                {
                    ymodem_Set_result(this, YMODEM_RESULT_PASSIVE_CANCEL);
                }
                else if (YMODEM_CODE_SOH == YMODEM_GET_HEADER() || YMODEM_CODE_STX == YMODEM_GET_HEADER())
                {
                    if (this->cbs.on_data)
                    {
                        ymodem_code_t code;
                        code = this->cbs.on_data(YMODEM_GET_DATA_PTR(), YMODEM_GET_DATA_SIZE());
                        switch (code)
                        {
                        case YMODEM_CODE_ACK:
                            this->data.recv_correct_cnt++;
                            this->data.nack_retry_cnt = 0;
                            ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                            break;
                        case YMODEM_CODE_NAK:
                            this->data.recv_error_cnt++;
                            ymodem_set_reply_code(this, YMODEM_CODE_NAK);
                            this->data.nack_retry_cnt++;
                            break;
                        default:
                            ymodem_Set_result(this, YMODEM_RESULT_INITIATIVE_CANCEL);
                            break;
                        }
                    }
                    else
                    {
                        ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                    }
                }
                else if (YMODEM_CODE_EOT == YMODEM_GET_HEADER())
                {
                    ymodem_set_reply_code(this, YMODEM_CODE_NAK);
                    this->data.state = YMODEM_STATE_FINISHED;
                }
                else
                {
                    ymodem_Set_result(this, YMODEM_RESULT_ERROR_CODE);
                }
            }
        }
        break;

    case YMODEM_STATE_FINISHED:
        if (this->data.time_cnt++ >= YMODEM_TRANS_FINISH_TIME_MS)
        {
            ymodem_Set_result(this, YMODEM_RESULT_FINISH_TIMEOUT);
        }
        else
        {

            if (true == ymodem_get_data(this))
            {
                this->data.time_cnt = 0;
                this->data.packet_data_cnt = 0;
                if (YMODEM_CODE_SOH == YMODEM_GET_HEADER())
                {
                    if (this->cbs.on_end)
                    {
                        this->cbs.on_end(YMODEM_GET_DATA_PTR(), YMODEM_GET_DATA_SIZE());
                    }
                    ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                    ymodem_Set_result(this, YMODEM_RESULT_SUCESS);
                }
                else if (YMODEM_CODE_EOT == YMODEM_GET_HEADER())
                {
                    ymodem_set_reply_code(this, YMODEM_CODE_ACK);
                    ymodem_set_reply_code(this, YMODEM_CODE_C);
                }
                else
                {
                    ymodem_Set_result(this, YMODEM_RESULT_ERROR_CODE);
                }
            }
        }
        break;

    default:
        break;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @{
//
//*****************************************************************************

