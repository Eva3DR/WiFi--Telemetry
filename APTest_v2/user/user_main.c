/*
 * user_main.c
 *
 *      Author: elc
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/uart.h"
#include <mem.h>
#include <user_interface.h>
#include <espconn.h>
#include <ip_addr.h>
#include <user_esp_platform.h>
#include <usr_cmd.h>
#include <user_config.h>
#include <CircularBuffer.h>


#ifdef DEBUG_MODE
#define DEBUG(s)	ets_uart_printf(s)
#else
#define DEBUG(s)
#endif


#define EVENT_TASK_PRIO	2
#define EVENT_QUEUE_LEN 4
os_event_t *event_task_Queue;


LOCAL uint8_t endpoint_ip[4] = {0};
LOCAL uint32_t endpoint_port = 0;

#define STA_AP_OPER_MODE	3

#define AP_OPER_MODE		2
#define DEFAULT_SAP_SSID	"test56g"
#define DEFAULT_SAP_PSW		"1234567890"
#define DEFAULT_LOCAL_PORT	8080
#define DEFAULT_BAUDRATE	57600;
#define DEFAULT_RF_TX_PWR	81;
#define	DEFAULT_IP_1 		192
#define	DEFAULT_IP_2		168
#define	DEFAULT_IP_3		4
#define	DEFAULT_IP_4		1

LOCAL struct softap_config s_ap_config;
LOCAL struct espconn udp_conf;
LOCAL esp_udp udp_settings;


LOCAL void error_manage();

#define UDP_RX		1
#define SERIAL_RX	0
LOCAL char cmd_buf[128];
#define CMD_BUFFER_LENGTH	64
LOCAL uint8_t buf_contains_at();

#define UDP_MAX_PCK_LENGTH		256
#define UDP_RECV_BUFF_LENGTH	1024
circular_buffer_t udp_rx_cbuf;
#define UDP_BUFFER_SIZE	512
LOCAL char rx_buffer[UDP_BUFFER_SIZE];
LOCAL uint8_t rx_data_length;
#define SERIAL_BUFFER_SIZE	512//1024
LOCAL uint8_t serial_buffer[SERIAL_BUFFER_SIZE];


LOCAL uint8_t udp_data_recv_flag = 0;
LOCAL uint8_t udp_cmd_recv_flag = 0;
LOCAL uint8_t udp_data_sent_flag = 0;
LOCAL uint32_t serial_data_avbl_flag = 0;
LOCAL uint8_t station_connected_flag = 0;

LOCAL uint8_t udp_setup();

#define	CONF_STORAGE_ADDR	0x3D
#define CONF_STORAGE_DATA_LENGTH sizeof(baud_rfPower_storage_t)
#define M_CODE	"xMPy"

typedef struct {
	uint32_t baudrate;
	uint32_t rf_power;
	uint32_t port;
	uint8_t ip[4];
	uint32_t mcode;
}baud_rfPower_storage_t;


LOCAL baud_rfPower_storage_t baud_rf_data;
struct esp_platform_saved_param  esp_param;

LOCAL void ICACHE_FLASH_ATTR store_conf();
LOCAL void ICACHE_FLASH_ATTR read_conf();
LOCAL void ICACHE_FLASH_ATTR scan_cb (void *arg, STATUS status);

LOCAL uint8_t os_e_memcmp(uint8_t *des, uint8_t *src, size_t n);



LOCAL uint8_t buf_contains_at(circular_buffer_t * c_buf) {
	uint8_t buf[128];//uint8_t buf[BUFFER_LENGTH+1];
	uint32_t length, i = 0;
	char * ptr;

	length = getSize(c_buf);
	length = copy(c_buf, buf);
	buf[length] = 0;

	if( ( ptr = strstr(buf,"AT+")) != NULL) {
		if( strstr(ptr,"\r\n") != NULL)
			return 1;
	}
	return 0;
}


LOCAL void ICACHE_FLASH_ATTR cmd_exec(uint8_t udp_rx)
{
	uint8_t idx = 0, f = 0, i = 0, j = 0;
	uint32_t baud = 0;
	uint8_t rf_pw = 0;
	char * init_ptr;
	char * end_ptr;
	char * aux_ptr;
	struct ip_info info;
	char cmd_buf_aux[64];

	if(udp_rx) {
		rx_data_length = read(&udp_rx_cbuf,rx_buffer,rx_data_length);
		rx_buffer[rx_data_length] = 0;
	}
	else {
		rx_data_length = os_strlen(cmd_buf);
		memcpy(rx_buffer,cmd_buf,rx_data_length);
		rx_buffer[rx_data_length] = 0;
	}

	os_memset(cmd_buf, 0, CMD_BUFFER_LENGTH);
	os_memset(cmd_buf_aux, 0, CMD_BUFFER_LENGTH);

	DEBUG(rx_buffer);

	while( idx < Cnt && !f) {
		if(strstr(rx_buffer,cmd_lst[idx]) != NULL) {
			f = 1;
		}
		else
			idx++;
	}
	if(f == 0) {
		return;
	}

	init_ptr = rx_buffer+3;
	init_ptr += strlen(cmd_lst[idx]) + 1;

	if( (end_ptr = strchr(&rx_buffer[idx],'\r')) == NULL )
		return;

	switch(idx) {
	case CWSAP:
		init_ptr++;
		aux_ptr = strstr(init_ptr,"\"");
		if(aux_ptr == NULL)
			return;
		os_memcpy(cmd_buf,init_ptr,aux_ptr - init_ptr);
		init_ptr = aux_ptr + 3;
		aux_ptr = strstr(init_ptr,"\"");
		if(aux_ptr == NULL)
			return;
		os_memcpy(cmd_buf_aux,init_ptr,aux_ptr - init_ptr);

		os_memset(s_ap_config.ssid, 0, sizeof(s_ap_config.ssid));
		os_memset(s_ap_config.password, 0, sizeof(s_ap_config.password));

		os_memcpy(s_ap_config.ssid,cmd_buf,os_strlen(cmd_buf));
		os_memcpy(s_ap_config.password,cmd_buf_aux,os_strlen(cmd_buf_aux));
		s_ap_config.ssid_len = os_strlen(cmd_buf);

		wifi_softap_set_config (&s_ap_config);
		uart0_tx_buffer("DONE!\r\n", 7);
		break;
	case UART:
		os_memcpy(cmd_buf,init_ptr,end_ptr - init_ptr);
		baud = atoi(cmd_buf);
#ifdef DEBUG_MODE
		os_sprintf(cmd_buf_aux,"Baud:%d.\n\r",baud);
#endif
		DEBUG(cmd_buf_aux);
		uart_change_baud(baud,baud);
		baud_rf_data.baudrate = baud;
		store_conf();
		break;
	case RF_PWR:
		os_memcpy(cmd_buf,init_ptr,end_ptr - init_ptr);
		rf_pw = atoi(cmd_buf);
#ifdef DEBUG_MODE
		os_sprintf(cmd_buf_aux,"RX_PWR:%d.\n\r",rf_pw);
#endif
		DEBUG(cmd_buf_aux);

		if(rf_pw <= 82) {
			system_phy_set_max_tpw(rf_pw);
			baud_rf_data.rf_power = rf_pw;
			store_conf();
		}
		break;
	case BAUD_TESTING:
		uart0_tx_buffer(TESTING_MSG, os_strlen(TESTING_MSG));
		break;
	case CIPPORT:
		init_ptr++;
		aux_ptr = strstr(init_ptr,"\"");
		if(aux_ptr == NULL)
			return;

		os_memcpy(cmd_buf,init_ptr,aux_ptr - init_ptr);
		os_memset(baud_rf_data.ip,0,4);
		f = os_strlen(cmd_buf);
		i = 0;
		while(j < f) {
			if( cmd_buf[j] == '.' ) {
				i++;
			}
			else {
				baud_rf_data.ip[i] *= 10;
				baud_rf_data.ip[i] += cmd_buf[j] - '0';
			}
			j++;
		}
#ifdef DEBUG_MODE
		os_sprintf(cmd_buf,"IP:%d.%d.%d.%d\r\n",baud_rf_data.ip[0],baud_rf_data.ip[1],baud_rf_data.ip[2],baud_rf_data.ip[3]);
#endif
		DEBUG(cmd_buf);

		init_ptr = aux_ptr+2;
		os_memcpy( cmd_buf_aux,init_ptr,end_ptr - init_ptr);
		baud_rf_data.port = atoi(cmd_buf_aux);

#ifdef DEBUG_MODE
		os_sprintf(cmd_buf,"Port:%d.\r\n",baud_rf_data.port);
#endif
		DEBUG(cmd_buf);

		wifi_softap_dhcps_stop();
	    IP4_ADDR(&info.ip, baud_rf_data.ip[0], baud_rf_data.ip[1], baud_rf_data.ip[2], baud_rf_data.ip[3]);
	    IP4_ADDR(&info.gw, baud_rf_data.ip[0], baud_rf_data.ip[1], baud_rf_data.ip[2], baud_rf_data.ip[3]);
	    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	    wifi_set_ip_info(SOFTAP_IF, &info);
	    wifi_softap_dhcps_start();

	    espconn_delete(&udp_conf);
	    udp_settings.local_port = baud_rf_data.port;
	    udp_conf.proto.udp = &udp_settings;
	    udp_setup();
	    store_conf();
#if 0
	    if( wifi_get_ip_info(SOFTAP_IF,&info)) {
	    	os_sprintf(cmd_buf,"IPN:%d.%d.%d.%d\r\n",ip4_addr1(&info.ip.addr),ip4_addr2(&info.ip.addr),ip4_addr3(&info.ip.addr),ip4_addr4(&info.ip.addr));
	    	DEBUG(cmd_buf);
	    }
#endif
		break;
	case SCAN:		// only on STATION + AP mode
		if( wifi_station_scan (NULL, scan_cb) )
			ets_uart_printf("Call scan\r\n");
		else
			ets_uart_printf("Scan call fail");
		break;
	}
}


LOCAL void ICACHE_FLASH_ATTR scan_cb (void *arg, STATUS status) {
	if (status != OK)
		return;

	struct bss_info *bss_link = (struct bss_info *)arg;

	while((bss_link = bss_link->next.stqe_next) != NULL) { //ignore first
		ets_uart_printf("SSID:%s\r\n",bss_link->ssid);
		ets_uart_printf("BSSID:%02X:%02X:%02X:%02X:%02X:%02X\r\n",bss_link->bssid[0],
				bss_link->bssid[1],bss_link->bssid[2],bss_link->bssid[3],bss_link->bssid[4],
				bss_link->bssid[5]);
		ets_uart_printf("RSSI:%d\r\n",bss_link->rssi);
	}

}


LOCAL void ICACHE_FLASH_ATTR sent_cb (void *arg) {
	udp_data_sent_flag = 1;
}

void wifi_handle_event_cb(System_Event_t *evt) {
	switch(evt->event) {
	case EVENT_SOFTAPMODE_STACONNECTED:
		DEBUG("STATION_CONN");
		station_connected_flag = 1;
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		break;
	}
}



LOCAL void ICACHE_FLASH_ATTR recv_cb(void* arg, char* pdata, unsigned short len) {
	udp_settings.local_port = baud_rf_data.port;
	udp_conf.proto.udp = &udp_settings;
	struct espconn * endpoint_info = (struct espconn *)arg;
	memcpy(endpoint_ip,endpoint_info->proto.udp->remote_ip,4);
	endpoint_port = endpoint_info->proto.udp->remote_port;

	write(&udp_rx_cbuf,pdata,len);
	if(buf_contains_at(&udp_rx_cbuf))
		udp_cmd_recv_flag = 1;
	else
		udp_data_recv_flag = 1;

	rx_data_length = len;
}

LOCAL void ICACHE_FLASH_ATTR udp_send(uint8_t * data)
{
	udp_settings.local_port = baud_rf_data.port;
	udp_settings.remote_port = endpoint_port;
	udp_conf.proto.udp = &udp_settings;
	os_memcpy(udp_settings.remote_ip,endpoint_ip,4);
	espconn_send(&udp_conf, (uint8 *)data, serial_data_avbl_flag);
	serial_data_avbl_flag = 0;
}


LOCAL void ICACHE_FLASH_ATTR event_task(os_event_t *e) {
	static uint16_t buff_data_avb_flag = 0;
	static uint16_t buff_data_avb_idx = 0;
	uint32_t length = 0;

	system_soft_wdt_feed();
	if((length = uart0_rx_mav_available()) > 0) {
		length = uart0_rx_data_available();
		if(length > UDP_MAX_PCK_LENGTH) {
			length = UDP_MAX_PCK_LENGTH;
		}
		serial_data_avbl_flag = length;
		uart0_rx_gets_len(serial_buffer, length);
		udp_send(serial_buffer);
		if(baud_rf_data.baudrate == 921600)
			os_delay_us(20);
		os_memset(serial_buffer,0,SERIAL_BUFFER_SIZE);
	}
	if(udp_data_recv_flag) {
		udp_data_recv_flag = 0;
		rx_data_length = getSize(&udp_rx_cbuf);
		rx_data_length = read(&udp_rx_cbuf,rx_buffer,rx_data_length);
		rx_buffer[rx_data_length] = 0;
		uart0_tx_buffer(rx_buffer, rx_data_length);
	}
	if(udp_cmd_recv_flag) {
		udp_cmd_recv_flag = 0;
		cmd_exec(UDP_RX);
	}
	if( uart0_rx_command_available() ) {
		uart0_rx_get_command(cmd_buf);
		cmd_exec(SERIAL_RX);

		uart0_tx_buffer("DONE\r\n", 6);
	}

	system_os_post(EVENT_TASK_PRIO, 0, 0 );
}


void user_rf_pre_init(void) {
}


void user_init(void)
{
	const char * err_op_mode = "AP_MODE_FAIL\r\n";
	const char * err_udp = "UDP_FAIL\r\n";
	uint8_t dev_conf = 0;

	struct ip_info info;

	create(&udp_rx_cbuf, UDP_RECV_BUFF_LENGTH);

	wifi_softap_get_config(&s_ap_config);

	udp_conf.type = ESPCONN_UDP;
	udp_conf.state=ESPCONN_NONE;
	udp_settings.remote_port = endpoint_port;
	os_memcpy(udp_settings.remote_ip,endpoint_ip,4);


	os_delay_us(300);
	read_conf();
	os_delay_us(300);


	if(os_e_memcmp((uint8_t *)&baud_rf_data.mcode,M_CODE,os_strlen(M_CODE)) != 0) {
		uart_init(BIT_RATE_57600,BIT_RATE_57600);
		baud_rf_data.baudrate = DEFAULT_BAUDRATE;
	    baud_rf_data.rf_power = DEFAULT_RF_TX_PWR;
	    baud_rf_data.port = DEFAULT_LOCAL_PORT;
	    baud_rf_data.ip[0] = DEFAULT_IP_1;	baud_rf_data.ip[1] = DEFAULT_IP_2;
	    baud_rf_data.ip[2] = DEFAULT_IP_3;	baud_rf_data.ip[3] = DEFAULT_IP_4;

		store_conf();

		os_memset(s_ap_config.ssid, 0, sizeof(s_ap_config.ssid));
		os_memset(s_ap_config.password, 0, sizeof(s_ap_config.password));
		os_memcpy(s_ap_config.ssid,DEFAULT_SAP_SSID,os_strlen(DEFAULT_SAP_SSID));
		os_memcpy(s_ap_config.password,DEFAULT_SAP_PSW,os_strlen(DEFAULT_SAP_PSW));
		s_ap_config.ssid_len = os_strlen(DEFAULT_SAP_SSID);
		s_ap_config.authmode = AUTH_WPA_WPA2_PSK;
		dev_conf = 1;
	}
	else {
		uart_init(baud_rf_data.baudrate,baud_rf_data.baudrate);
		system_phy_set_max_tpw(baud_rf_data.rf_power);
	}

	udp_settings.local_port = baud_rf_data.port;
	udp_conf.proto.udp = &udp_settings;

	wifi_softap_dhcps_stop();
    IP4_ADDR(&info.ip, baud_rf_data.ip[0], baud_rf_data.ip[1], baud_rf_data.ip[2], baud_rf_data.ip[3]);
    IP4_ADDR(&info.gw, baud_rf_data.ip[0], baud_rf_data.ip[1], baud_rf_data.ip[2], baud_rf_data.ip[3]);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    wifi_set_ip_info(SOFTAP_IF, &info);
    wifi_softap_dhcps_start();

	system_soft_wdt_feed();

	if(!wifi_set_opmode (AP_OPER_MODE)) {
		error_manage();
	}
	else {
		if(dev_conf) {
			if(!wifi_softap_set_config (&s_ap_config))
				error_manage();
		}
		udp_setup();
	}

	wifi_set_event_handler_cb(wifi_handle_event_cb);

	event_task_Queue=(os_event_t *)os_malloc(sizeof(os_event_t) * EVENT_QUEUE_LEN);
	system_os_task(event_task, EVENT_TASK_PRIO, event_task_Queue, EVENT_QUEUE_LEN);
	system_os_post(EVENT_TASK_PRIO, 0, 0 );
}


LOCAL void ICACHE_FLASH_ATTR clean_set_struct(void) {
	os_memset(esp_param.devkey,0,DEVKEY_LENGTH);
	os_memset(esp_param.token,0,DEVKEY_LENGTH);

	os_memcpy(&baud_rf_data.mcode,M_CODE,os_strlen(M_CODE));
	os_memcpy(esp_param.token,&baud_rf_data,CONF_STORAGE_DATA_LENGTH);
}


LOCAL void ICACHE_FLASH_ATTR store_conf() {
#if 0
	ETS_UART_INTR_DISABLE();
#endif
	esp_param.activeflag = 1;
	clean_set_struct();
	system_param_save_with_protect(CONF_STORAGE_ADDR, &esp_param, sizeof(esp_param));
#if 0
	ETS_UART_INTR_ENABLE();
#endif
}


LOCAL void ICACHE_FLASH_ATTR read_conf() {

	clean_set_struct();
	system_param_load(CONF_STORAGE_ADDR, 0, &esp_param, sizeof(esp_param));
	os_memcpy(&baud_rf_data,esp_param.token,CONF_STORAGE_DATA_LENGTH);
}

LOCAL void error_manage() {
	system_restart();
}

LOCAL uint8_t os_e_memcmp(uint8_t *des, uint8_t *src, size_t n) {
	uint32_t i;
	for(i = 0 ; i < n; i++) {
		if( des[i] != src[i] )
			return 1;
	}
	return 0;
}


LOCAL uint8_t udp_setup() {
	if(espconn_regist_sentcb(&udp_conf,sent_cb) != 0) {
		return 1;
	}
	else {
		if(espconn_regist_recvcb(&udp_conf,recv_cb) != 0)
			return 1;

		else if(espconn_create(&udp_conf) != 0)
			return 1;
	}
	return 0;
}

