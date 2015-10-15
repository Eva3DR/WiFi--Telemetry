///******************************************************************************
// * Copyright 2013-2014 Espressif Systems (Wuxi)
// *
// * FileName: uart.c
// *
// * Description: Two UART mode configration and interrupt handler.
// *              Check your hardware connection while use this mode.
// *
// * Modification history:
// *     2014/3/12, v1.0 create this file.
//*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"
#include "user_config.h"
#include <CircularBuffer.h>

#define UART0   0
#define UART1   1

// UartDev is defined and initialized in rom code.
extern UartDevice UartDev;

#define UART_RECV_BUFF_LENGTH	4096
circular_buffer_t rx_cbuf;

LOCAL volatile uint8_t state_cmd = 0;
LOCAL volatile uint8_t state_mav = 0;
LOCAL volatile uint32_t mav_rcv_length = 0;
LOCAL volatile uint32_t mav_pend_lenght = 0;
LOCAL volatile uint32_t mav_pend_lenght_rep = 0;
LOCAL volatile uint16_t mav_rcv_payload_length = 0;
LOCAL volatile uint8_t serial_mav_recv_flag = 0;
#define MAV_FRAME_HEADER	0xFE

enum serial_cmd_rcv_mav_state_t {
	MAV_HEADER = 0,
	MAV_PAYLOAD_LENGTH,
	MAV_IDS,
	MAV_PAYLOAD,
	MAV_CKS,
};


enum serial_cmd_rcv_state_t {
	CMD_INIT1 = 0,
	CMD_INIT2,
	CMD_INIT3,
	CMD_WRITE
};


LOCAL const char CMD_INIT[] = "AT+";
LOCAL char serial_command_rcv[APNAME_MAX_LENGTH+APPSW_MAX_LENGTH+12];
LOCAL volatile uint8_t cmd_pos = 0;
LOCAL volatile uint8_t serial_cmd_recv_flag = 0;

LOCAL void uart0_rx_intr_handler(void *para);

#define ENABLE_UART0_RX_INT			SET_PERI_REG_MASK(UART_INT_ENA(UART1), UART_RXFIFO_FULL_INT_ENA)
#define DISABLE_UART0_RX_INT		WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff)

/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart_config(uint8 uart_no)
{
    if (uart_no == UART1) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
    } else {
        /* rcv_buff size if 0x100 */
        ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    }

    uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));

    WRITE_PERI_REG(UART_CONF0(uart_no),    UartDev.exist_parity
                   | UartDev.parity
                   | (UartDev.stop_bits << UART_STOP_BIT_NUM_S)
                   | (UartDev.data_bits << UART_BIT_NUM_S));


    //clear rx and tx fifo,not ready
    SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

    //set rx fifo trigger
    WRITE_PERI_REG(UART_CONF1(uart_no), (UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S);

    //clear all interrupt
    WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
    //enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
*******************************************************************************/
LOCAL STATUS ICACHE_FLASH_ATTR
uart1_tx_one_char(uint8 TxChar)
{
    while (true)
	{
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(UART1)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
			break;
		}
	}

	WRITE_PERI_REG(UART_FIFO(UART1) , TxChar);
	return OK;
}

/******************************************************************************
 * FunctionName : uart1_write_char
 * Description  : Internal used function
 *                Do some special deal while tx char is '\r' or '\n'
 * Parameters   : char c - character to tx
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart1_write_char(char c)
{
    if (c == '\n') {
        uart1_tx_one_char('\r');
        uart1_tx_one_char('\n');
    } else if (c == '\r') {
    } else {
        uart1_tx_one_char(c);
    }
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
*******************************************************************************/
LOCAL void
uart0_rx_intr_handler(void *para)
{
    /* uart0 and uart1 intr combine togther, when interrupt occur, see reg 0x3ff20020, bit2, bit0 represents
     * uart1 and uart0 respectively
     */
    RcvMsgBuff *pRxBuff = (RcvMsgBuff *)para;
    uint8 RcvChar;

    if (UART_RXFIFO_FULL_INT_ST != (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
        return;
    }

    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);

    while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
        RcvChar = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

        *(pRxBuff->pWritePos) = RcvChar;
		
		/* you can add your handle code below.*/

    	switch(state_mav) {
		case MAV_HEADER:
						if(RcvChar == MAV_FRAME_HEADER) {
							state_mav = MAV_PAYLOAD_LENGTH;
							write(&rx_cbuf,&RcvChar,1);
							mav_rcv_length++;
						}
						break;
		case MAV_PAYLOAD_LENGTH:
						state_mav = MAV_IDS;
						mav_rcv_payload_length = RcvChar;
						write(&rx_cbuf,&RcvChar,1);
						mav_rcv_length++;
						break;
		case MAV_IDS:
						write(&rx_cbuf,&RcvChar,1);
						mav_rcv_length++;
						if(mav_rcv_length == 6 )
							state_mav = MAV_PAYLOAD;
						break;
		case MAV_PAYLOAD:
						write(&rx_cbuf,&RcvChar,1);
						mav_rcv_length++;
						if(mav_rcv_length == (mav_rcv_payload_length+6))
							state_mav = MAV_CKS;
						break;
		case MAV_CKS:
						write(&rx_cbuf,&RcvChar,1);
						mav_rcv_length++;
						if(mav_rcv_length == (mav_rcv_payload_length+6+2)) {
							state_mav = MAV_HEADER;
							serial_mav_recv_flag = 1;
							mav_pend_lenght += mav_rcv_length;
							mav_pend_lenght_rep = mav_pend_lenght;
							mav_rcv_length = 0;
						}
						break;
		}

        switch(state_cmd) {
        case CMD_INIT1: if(RcvChar == CMD_INIT[0])
        				state_cmd = CMD_INIT2;
        				break;
        case CMD_INIT2: if(RcvChar == CMD_INIT[1])
						state_cmd = CMD_INIT3;
						else
							state_cmd = CMD_INIT1;
						break;
        case CMD_INIT3:	if(RcvChar == CMD_INIT[2]) {
							memcpy(serial_command_rcv,CMD_INIT,3);
							state_cmd = CMD_WRITE;
							cmd_pos = 3;
						}
						else
							state_cmd = CMD_INIT1;
						break;
        case CMD_WRITE: if(RcvChar == '\r') {
        	        		state_cmd = CMD_INIT1;
        	        		serial_command_rcv[cmd_pos++] = RcvChar;
        	        		serial_command_rcv[cmd_pos] = 0;
        	        		cmd_pos = 0;
        	        		serial_cmd_recv_flag = 1;
        				}
						serial_command_rcv[cmd_pos++] = RcvChar;
						break;

        }

        // insert here for get one command line from uart
        if (RcvChar == '\r') {
            pRxBuff->BuffState = WRITE_OVER;
        }

        pRxBuff->pWritePos++;

		// if we hit the end of the buffer, loop back to the beginning
        if (pRxBuff->pWritePos == (pRxBuff->pRcvMsgBuff + RX_BUFF_SIZE)) {
            // overflow ...we may need more error handle here.
            pRxBuff->pWritePos = pRxBuff->pRcvMsgBuff ;
        }
    }
}

ICACHE_FLASH_ATTR uint32_t uart0_rx_mav_available() {
	uint64_t len = 0;

#if 0
	ETS_UART_INTR_DISABLE();
#endif
	if(serial_mav_recv_flag) {
		serial_mav_recv_flag = 0;
		do {
			len =  mav_pend_lenght;
		} while(len != mav_pend_lenght);
		mav_pend_lenght = 0;
		if(mav_pend_lenght_rep == mav_pend_lenght)
			len +=  mav_pend_lenght;
		else
			mav_pend_lenght_rep = 0;
	}
#if 0
	ETS_UART_INTR_ENABLE();
#endif

	return len;
}

ICACHE_FLASH_ATTR uint32_t uart0_rx_mav_lenght() {
	return mav_pend_lenght;
}


ICACHE_FLASH_ATTR uint8_t uart0_rx_command_available() {
	if(serial_cmd_recv_flag) {
		serial_cmd_recv_flag = 0;
		return 1;
	}
	return 0;
}


ICACHE_FLASH_ATTR void uart0_rx_get_command(char * data) {
	memcpy(data,serial_command_rcv,os_strlen(serial_command_rcv));
}

ICACHE_FLASH_ATTR uint32_t uart0_rx_data_available() {
	return getSize(&rx_cbuf);
}

ICACHE_FLASH_ATTR int uart0_rx_one_char() {
	uint8_t ret;

	read(&rx_cbuf, &ret, 1);

	return ret;
}

ICACHE_FLASH_ATTR uint32_t uart0_rx_gets(uint8_t * data) {
	uint32_t length;

	length = getSize(&rx_cbuf);
	length = read(&rx_cbuf, data, length);

	return length;
}

ICACHE_FLASH_ATTR uint32_t uart0_rx_gets_len(uint8_t * data, uint64_t length) {
	length = read(&rx_cbuf, data, length);
	return length;
}








/******************************************************************************
 * FunctionName : uart0_tx_buffer
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart0_tx_buffer(uint8 *buf, uint16 len)
{
    uint16 i;

    for (i = 0; i < len; i++) {
        uart_tx_one_char(buf[i]);
    }
}

/******************************************************************************
 * FunctionName : uart_init
 * Description  : user interface for init uart
 * Parameters   : UartBautRate uart0_br - uart0 bautrate
 *                UartBautRate uart1_br - uart1 bautrate
 * Returns      : NONE
*******************************************************************************/

void ICACHE_FLASH_ATTR
uart_init(UartBautRate uart0_br, UartBautRate uart1_br)
{
    // rom use 74880 baut_rate, here reinitialize
    UartDev.baut_rate = uart0_br;
    uart_config(UART0);
    UartDev.baut_rate = uart1_br;
    uart_config(UART1);
    create(&rx_cbuf, UART_RECV_BUFF_LENGTH);
    ETS_UART_INTR_ENABLE();

    // install uart1 putc callback
    os_install_putc1((void *)uart1_write_char);
//    UartDev.rcv_buff.pWritePos = UartDev.rcv_buff.pRcvMsgBuff;
//    UartDev.rcv_buff.pReadPos  = UartDev.rcv_buff.pRcvMsgBuff;

}


void ICACHE_FLASH_ATTR _uart_init(void) {
	uart_config(UART0);
	uart_config(UART1);
	ETS_UART_INTR_ENABLE();

    // install uart1 putc callback
    os_install_putc1((void *)uart1_write_char);
}


void ICACHE_FLASH_ATTR
uart_change_baud(UartBautRate uart0_br, UartBautRate uart1_br)
{
	UartDev.baut_rate = uart0_br;
	uart_config(UART0);
	UartDev.baut_rate = uart1_br;
	uart_config(UART1);
}


