/*
********************************************************************************
File Include Section
********************************************************************************
*/

#include "jsinteractive.h"
#include "jshardware.h"
#include "jsparse.h"
#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "dhcp.h"

/*
********************************************************************************
Define Part
********************************************************************************
*/


/*
********************************************************************************
Local Variable Declaration Section
********************************************************************************
*/
// FIXME these should all be stored on the stack

extern uint8_t tx_mem_conf[8];
extern uint8_t rx_mem_conf[8];

uint8_t DHCP_SIP[4];
uint8_t DHCP_SHA[6];
uint8_t OLD_SIP[4];

int8_t dhcp_state;
int8_t retry_count;

un_l2cval lease_time;
JsSysTime next_time;

uint8_t DHCP_timeout;
uint32_t DHCP_XID;

RIP_MSG  MSG;

uint8_t HOST_NAME[] = "WIZnet";
uint8_t Cip[4] = {0,0,0,0};

extern void set_default_netinfo(void);

/*
********************************************************************************
Function Implementation Part
********************************************************************************
*/
/*
*********************************************************************************************************
*              SEND DHCP DISCOVER
*
* Description: This function sends DHCP DISCOVER message to DHCP server.
* Arguments  : s - is a socket number.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_DISCOVER(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
	uint16_t i;
	uint8_t ip[4];
	uint16_t k = 0;

	MSG.op = DHCP_BOOTREQUEST;
	MSG.htype = DHCP_HTYPE10MB;
	MSG.hlen = DHCP_HLENETHERNET;
	MSG.hops = DHCP_HOPS;
	MSG.xid = DHCP_XID;
	MSG.secs = DHCP_SECS;
	MSG.flags = DHCP_FLAGSBROADCAST;

	MSG.ciaddr[0] = 0;
	MSG.ciaddr[1] = 0;
	MSG.ciaddr[2] = 0;
	MSG.ciaddr[3] = 0;

	MSG.yiaddr[0] = 0;
	MSG.yiaddr[1] = 0;
	MSG.yiaddr[2] = 0;
	MSG.yiaddr[3] = 0;

	MSG.siaddr[0] = 0;
	MSG.siaddr[1] = 0;
	MSG.siaddr[2] = 0;
	MSG.siaddr[3] = 0;

	MSG.giaddr[0] = 0;
	MSG.giaddr[1] = 0;
	MSG.giaddr[2] = 0;
	MSG.giaddr[3] = 0;

	MSG.chaddr[0] = (*pWIZNETINFO).mac[0];
	MSG.chaddr[1] = (*pWIZNETINFO).mac[1];
	MSG.chaddr[2] = (*pWIZNETINFO).mac[2];
	MSG.chaddr[3] = (*pWIZNETINFO).mac[3];
	MSG.chaddr[4] = (*pWIZNETINFO).mac[4];
	MSG.chaddr[5] = (*pWIZNETINFO).mac[5];

	for (i = 6; i < 16; i++) MSG.chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG.sname[i] = 0;
	for (i = 0; i < 128; i++) MSG.file[i] = 0;

	// MAGIC_COOKIE
	MSG.OPT[k++] = 0x63;
	MSG.OPT[k++] = 0x82;
	MSG.OPT[k++] = 0x53;
	MSG.OPT[k++] = 0x63;

	// Option Request Param
	MSG.OPT[k++] = dhcpMessageType;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = DHCP_DISCOVER;
	
	// Client identifier
	MSG.OPT[k++] = dhcpClientIdentifier;
	MSG.OPT[k++] = 0x07;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = (*pWIZNETINFO).mac[0];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[1];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[2];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[3];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[4];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[5];
	
	// host name
	MSG.OPT[k++] = hostName;
	MSG.OPT[k++] = 9; // length of hostname
	MSG.OPT[k++] = HOST_NAME[0];
	MSG.OPT[k++] = HOST_NAME[1];
	MSG.OPT[k++] = HOST_NAME[2];
	MSG.OPT[k++] = HOST_NAME[3];
	MSG.OPT[k++] = HOST_NAME[4];
	MSG.OPT[k++] = HOST_NAME[5];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[3];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[4];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[5];


	MSG.OPT[k++] = dhcpParamRequest;
	MSG.OPT[k++] = 0x06;	// length of request
	MSG.OPT[k++] = subnetMask;
	MSG.OPT[k++] = routersOnSubnet;
	MSG.OPT[k++] = dns;
	MSG.OPT[k++] = domainName;
	MSG.OPT[k++] = dhcpT1value;
	MSG.OPT[k++] = dhcpT2value;
	MSG.OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG.OPT[i] = 0;

	// send broadcasting packet
	for (i = 0; i < 4; i++) ip[i] = 255;

	jsiConsolePrintf("> Send DHCP_DISCOVER\r\n");
	sendto(s, (uint8_t *)(&MSG.op), RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/*
*********************************************************************************************************
*              SEND DHCP REQUEST
*
* Description: This function sends DHCP REQUEST message to DHCP server.
* Arguments  : s - is a socket number.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_REQUEST(uint8_t s, uint8_t *Cip, uint8_t *d_addr, wiz_NetInfo *pWIZNETINFO)
{
	int i;
	uint8_t ip[10];
	uint16_t k = 0;

	MSG.op = DHCP_BOOTREQUEST;
	MSG.htype = DHCP_HTYPE10MB;
	MSG.hlen = DHCP_HLENETHERNET;
	MSG.hops = DHCP_HOPS;
	MSG.xid = DHCP_XID;
	MSG.secs = DHCP_SECS;
	//MSG.flags = DHCP_FLAGSBROADCAST;
	if (d_addr[0] == 0xff) 	MSG.flags = DHCP_FLAGSBROADCAST;
	else MSG.flags = 0;
	
	MSG.ciaddr[0] = Cip[0];
	MSG.ciaddr[1] = Cip[1];
	MSG.ciaddr[2] = Cip[2];
	MSG.ciaddr[3] = Cip[3];

	MSG.yiaddr[0] = 0;
	MSG.yiaddr[1] = 0;
	MSG.yiaddr[2] = 0;
	MSG.yiaddr[3] = 0;

	MSG.siaddr[0] = 0;
	MSG.siaddr[1] = 0;
	MSG.siaddr[2] = 0;
	MSG.siaddr[3] = 0;

	MSG.giaddr[0] = 0;
	MSG.giaddr[1] = 0;
	MSG.giaddr[2] = 0;
	MSG.giaddr[3] = 0;

	MSG.chaddr[0] = (*pWIZNETINFO).mac[0];
	MSG.chaddr[1] = (*pWIZNETINFO).mac[1];
	MSG.chaddr[2] = (*pWIZNETINFO).mac[2];
	MSG.chaddr[3] = (*pWIZNETINFO).mac[3];
	MSG.chaddr[4] = (*pWIZNETINFO).mac[4];
	MSG.chaddr[5] = (*pWIZNETINFO).mac[5];

	for (i = 6; i < 16; i++) MSG.chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG.sname[i] = 0;
	for (i = 0; i < 128; i++) MSG.file[i] = 0;

	// MAGIC_COOKIE 
	MSG.OPT[k++] = 0x63;
	MSG.OPT[k++] = 0x82;
	MSG.OPT[k++] = 0x53;
	MSG.OPT[k++] = 0x63;

	// Option Request Param.
	MSG.OPT[k++] = dhcpMessageType;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = DHCP_REQUEST;

	MSG.OPT[k++] = dhcpClientIdentifier;
	MSG.OPT[k++] = 0x07;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = (*pWIZNETINFO).mac[0];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[1];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[2];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[3];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[4];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[5];

	if (d_addr[0] == 0xff) {
		MSG.OPT[k++] = dhcpRequestedIPaddr;
		MSG.OPT[k++] = 0x04;
		MSG.OPT[k++] = (*pWIZNETINFO).ip[0];
		MSG.OPT[k++] = (*pWIZNETINFO).ip[1];
		MSG.OPT[k++] = (*pWIZNETINFO).ip[2];
		MSG.OPT[k++] = (*pWIZNETINFO).ip[3];
	
		MSG.OPT[k++] = dhcpServerIdentifier;
		MSG.OPT[k++] = 0x04;
		MSG.OPT[k++] = DHCP_SIP[0];
		MSG.OPT[k++] = DHCP_SIP[1];
		MSG.OPT[k++] = DHCP_SIP[2];
		MSG.OPT[k++] = DHCP_SIP[3];
	}

	// host name
	MSG.OPT[k++] = hostName;
	MSG.OPT[k++] = 9; // length of hostname
	MSG.OPT[k++] = HOST_NAME[0];
	MSG.OPT[k++] = HOST_NAME[1];
	MSG.OPT[k++] = HOST_NAME[2];
	MSG.OPT[k++] = HOST_NAME[3];
	MSG.OPT[k++] = HOST_NAME[4];
	MSG.OPT[k++] = HOST_NAME[5];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[3];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[4];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[5];
	
	MSG.OPT[k++] = dhcpParamRequest;
	MSG.OPT[k++] = 0x08;
	MSG.OPT[k++] = subnetMask;
	MSG.OPT[k++] = routersOnSubnet;
	MSG.OPT[k++] = dns;
	MSG.OPT[k++] = domainName;
	MSG.OPT[k++] = dhcpT1value;
	MSG.OPT[k++] = dhcpT2value;
	MSG.OPT[k++] = performRouterDiscovery;
	MSG.OPT[k++] = staticRoute;
	MSG.OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG.OPT[i] = 0;

	// send broadcasting packet
	for (i = 0; i < 4; i++) ip[i] = d_addr[i];

	jsiConsolePrintf("> Send DHCP_Request\r\n");
	
	sendto(s, (uint8_t *)(&MSG.op), RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);

}

/*
*********************************************************************************************************
*              SEND DHCP DHCPDECLINE
*
* Description: This function sends DHCP RELEASE message to DHCP server.
* Arguments  : s - is a socket number.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void send_DHCP_DECLINE(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
	int i;
	uint8_t ip[10];
	uint16_t k = 0;

	MSG.op = DHCP_BOOTREQUEST;
	MSG.htype = DHCP_HTYPE10MB;
	MSG.hlen = DHCP_HLENETHERNET;
	MSG.hops = DHCP_HOPS;
	MSG.xid = DHCP_XID;
	MSG.secs = DHCP_SECS;
	MSG.flags = 0;

	MSG.ciaddr[0] = 0;
	MSG.ciaddr[1] = 0;
	MSG.ciaddr[2] = 0;
	MSG.ciaddr[3] = 0;

	MSG.yiaddr[0] = 0;
	MSG.yiaddr[1] = 0;
	MSG.yiaddr[2] = 0;
	MSG.yiaddr[3] = 0;

	MSG.siaddr[0] = 0;
	MSG.siaddr[1] = 0;
	MSG.siaddr[2] = 0;
	MSG.siaddr[3] = 0;

	MSG.giaddr[0] = 0;
	MSG.giaddr[1] = 0;
	MSG.giaddr[2] = 0;
	MSG.giaddr[3] = 0;

	MSG.chaddr[0] = (*pWIZNETINFO).mac[0];
	MSG.chaddr[1] = (*pWIZNETINFO).mac[1];
	MSG.chaddr[2] = (*pWIZNETINFO).mac[2];
	MSG.chaddr[3] = (*pWIZNETINFO).mac[3];
	MSG.chaddr[4] = (*pWIZNETINFO).mac[4];
	MSG.chaddr[5] = (*pWIZNETINFO).mac[5];

	for (i = 6; i < 16; i++) MSG.chaddr[i] = 0;
	for (i = 0; i < 64; i++) MSG.sname[i] = 0;
	for (i = 0; i < 128; i++) MSG.file[i] = 0;

	// MAGIC_COOKIE
	MSG.OPT[k++] = 0x63;
	MSG.OPT[k++] = 0x82;
	MSG.OPT[k++] = 0x53;
	MSG.OPT[k++] = 0x63;

	// Option Request Param.
	MSG.OPT[k++] = dhcpMessageType;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = DHCP_DECLINE;

	MSG.OPT[k++] = dhcpClientIdentifier;
	MSG.OPT[k++] = 0x07;
	MSG.OPT[k++] = 0x01;
	MSG.OPT[k++] = (*pWIZNETINFO).mac[0];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[1];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[2];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[3];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[4];
	MSG.OPT[k++] = (*pWIZNETINFO).mac[5];

	MSG.OPT[k++] = dhcpRequestedIPaddr;
	MSG.OPT[k++] = 0x04;
	MSG.OPT[k++] = (*pWIZNETINFO).ip[0];
	MSG.OPT[k++] = (*pWIZNETINFO).ip[1];
	MSG.OPT[k++] = (*pWIZNETINFO).ip[2];
	MSG.OPT[k++] = (*pWIZNETINFO).ip[3];

	MSG.OPT[k++] = dhcpServerIdentifier;
	MSG.OPT[k++] = 0x04;
	MSG.OPT[k++] = DHCP_SIP[0];
	MSG.OPT[k++] = DHCP_SIP[1];
	MSG.OPT[k++] = DHCP_SIP[2];
	MSG.OPT[k++] = DHCP_SIP[3];

	MSG.OPT[k++] = endOption;

	for (i = k; i < OPT_SIZE; i++) MSG.OPT[i] = 0;

	//send broadcasting packet
	ip[0] = 255;
	ip[1] = 255;
	ip[2] = 255;
	ip[3] = 255;

//	jsiConsolePrintf("\r\n> send DHCP_Decline\r\n");

	sendto(s, (uint8_t *)(&MSG.op), RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/*
*********************************************************************************************************
*              PARSE REPLY MSG
*
* Description: This function parses the reply message from DHCP server.
* Arguments  : s      - is a socket number.
*              length - is a size data to receive.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
int8_t parseDHCPMSG(uint8_t s, uint16_t length, wiz_NetInfo *pWIZNETINFO)
{
	uint8_t svr_addr[6];
	uint16_t  svr_port;

	uint16_t i, len;
	uint8_t * p;
	uint8_t * e;
	uint8_t type, opt_len;

  //*************
 // Add James Kim for IWatchDog
#ifdef _IWDG
		/* Reload IWDG counter */
		IWDG_ReloadCounter();
#endif	  
//
//**************
	//len = recvfrom(s, (uint8_t *)&MSG.op, length, svr_addr, &svr_port);
	len = recvfrom(s, (uint8_t *)&MSG.op, length, svr_addr, &svr_port);

	if (svr_port == DHCP_SERVER_PORT) {

		for (i = 0; i < 6; i++)
			if (MSG.chaddr[i] != (*pWIZNETINFO).mac[i]) {
				type = 0;
				goto PARSE_END;
			}

		for (i = 0; i < 4; i++) {
			(*pWIZNETINFO).ip[i] = MSG.yiaddr[i];
		}
		
		type = 0;
		p = (uint8_t *)(&MSG.op);
		p = p + 240;
		e = p + (len - 240);

		while ( p < e ) {
  //*************
 // Add James Kim for IWatchDog
#ifdef _IWDG
		/* Reload IWDG counter */
		IWDG_ReloadCounter();
#endif	  
//
//**************
			switch ( *p ) {

			case endOption :
				goto PARSE_END;
       			case padOption :
				p++;
				break;
			case dhcpMessageType :
				p++;
				p++;
				type = *p++;
				break;
			case subnetMask :
				p++;
				p++;
				for (i = 0; i < 4; i++)	 (*pWIZNETINFO).sn[i] = *p++;
				break;
			case routersOnSubnet :
				p++;
				opt_len = *p++;       
				for (i = 0; i < 4; i++)	(*pWIZNETINFO).gw[i] = *p++;
				for (i = 0; i < (opt_len-4); i++) p++;
				break;
			
			case dns :
				p++;                  
				opt_len = *p++;       
				for (i = 0; i < 4; i++)	(*pWIZNETINFO).dns[i] = *p++;
				for (i = 0; i < (opt_len-4); i++) p++;
				break;
				
				
			case dhcpIPaddrLeaseTime :
				p++;
				opt_len = *p++;
				lease_time.cVal[3] = *p++;
				lease_time.cVal[2] = *p++;
				lease_time.cVal[1] = *p++;
				lease_time.cVal[0] = *p++;
				break;

			case dhcpServerIdentifier :
				p++;
				opt_len = *p++;
				DHCP_SIP[0] = *p++;
				DHCP_SIP[1] = *p++;
				DHCP_SIP[2] = *p++;
				DHCP_SIP[3] = *p++;
				break;

			default :
				p++;
				opt_len = *p++;
				p += opt_len;
				break;
			} // switch
		} // while
	} // if

PARSE_END :
	return	type;
}

/*
*********************************************************************************************************
*              CHECK DHCP STATE
*
* Description: This function checks the state of DHCP.
* Arguments  : None.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void check_DHCP_state(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
	uint16_t len, i;
	uint8_t type, flag;
	uint8_t d_addr[4];
	
	type = 0;
	
	if ((len = getSn_RX_RSR(s)) > 0) {
		type = parseDHCPMSG(s, len, pWIZNETINFO);
	}
	switch ( dhcp_state ) {
		case STATE_DHCP_DISCOVER :
			if (type == DHCP_OFFER) {
				jsiConsolePrintf("> Receive DHCP_OFFER\r\n");
				
				for (i = 0; i < 4; i++) d_addr[i] = 0xff;
				send_DHCP_REQUEST(s, Cip, d_addr, pWIZNETINFO);
				
				dhcp_state = STATE_DHCP_REQUEST;
			} else check_Timeout(s, pWIZNETINFO);
		break;

		case STATE_DHCP_REQUEST :
			if (type == DHCP_ACK) {
				

				jsiConsolePrintf("> Receive DHCP_ACK\r\n");
				
				if (check_leasedIP(s, pWIZNETINFO)) {
					//iinchip_init() - WIZnet chip reset & Delay (10ms);
					//Set_network();
					ctlwizchip(CW_RESET_WIZCHIP, 0);
					ctlnetwork(CN_SET_NETINFO, (void*)&(*pWIZNETINFO));
					/*
					for (i = 0; i < 12; i++) {
						
						//EEP_Write(EEP_LIP+i, *(uint8_t *)((*pWIZNETINFO).ip+i));
						
					}
					*/

					next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
					retry_count = 0;
					dhcp_state = STATE_DHCP_LEASED;
				} else {

					next_time = jshGetSystemTime() + DHCP_WAIT_TIME1;
					retry_count = 0;
					//dhcp_state = STATE_DHCP_DISCOVER;
					dhcp_state = STATE_DHCP_LEASED;
					jsiConsolePrintf("> => Recceived IP is invalid\r\n");
					
					//iinchip_init();
					//Set_Default();
					//Set_network();
					//ctlwizchip(CW_RESET_WIZCHIP, 0);
					//set_default_netinfo();
					//ctlnetwork(CN_SET_NETINFO, (void*)&(*pWIZNETINFO));
				}
			} else if (type == DHCP_NAK) {
				jsiConsolePrintf("> Receive DHCP_NACK\r\n");

				next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
				retry_count = 0;
				dhcp_state = STATE_DHCP_DISCOVER;
			} else check_Timeout(s, pWIZNETINFO);
		break;

		case STATE_DHCP_LEASED :
			if ((lease_time.lVal != 0xffffffff) && ((lease_time.lVal/2) < jshGetSystemTime())) {
				
				jsiConsolePrintf("> Renewal IP address \r\n");

				type = 0;
				for (i = 0; i < 4; i++)	OLD_SIP[i] = (*pWIZNETINFO).ip[i];
				for (i = 0; i < 4; i++)	d_addr[i] = DHCP_SIP[i];
				
				DHCP_XID++;
				send_DHCP_REQUEST(s, (*pWIZNETINFO).ip, d_addr, pWIZNETINFO);
				dhcp_state = STATE_DHCP_REREQUEST;


				next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
			}
		break;

		case STATE_DHCP_REREQUEST :
			if (type == DHCP_ACK) {
				retry_count = 0;
				flag = 0;
				for (i = 0; i < 4; i++)	{
					if (OLD_SIP[i] != (*pWIZNETINFO).ip[i]) {
						flag = 1;
						break;
					}
				}
				
				// change to new IP address
				if (flag) {
					//iinchip_init();
					//Set_network();
					ctlwizchip(CW_RESET_WIZCHIP, 0);
					ctlnetwork(CN_SET_NETINFO, (void*)&(*pWIZNETINFO));
				}


				next_time = jshGetSystemTime() + DHCP_WAIT_TIME;

				dhcp_state = STATE_DHCP_LEASED;
			} else if (type == DHCP_NAK) {

				next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
				retry_count = 0;
				dhcp_state = STATE_DHCP_DISCOVER;
//				jsiConsolePrintf("state : STATE_DHCP_DISCOVER\r\n");
			} else check_Timeout(s, pWIZNETINFO);
		break;

		case STATE_DHCP_RELEASE :
		break;

		default :
		break;
	}
}

/*
*********************************************************************************************************
*              CHECK TIMEOUT
*
* Description: This function checks the timeout of DHCP in each state.
* Arguments  : None.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
void check_Timeout(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
	uint8_t i, d_addr[4];
	
	if (retry_count < MAX_DHCP_RETRY) {
		if (next_time < jshGetSystemTime()) {
			next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
			retry_count++;
			switch ( dhcp_state ) {
				case STATE_DHCP_DISCOVER :
//					jsiConsolePrintf("<<timeout>> state : STATE_DHCP_DISCOVER\r\n");
					send_DHCP_DISCOVER(s, pWIZNETINFO);
				break;
		
				case STATE_DHCP_REQUEST :
//					jsiConsolePrintf("<<timeout>> state : STATE_DHCP_REQUEST\r\n");

					for (i = 0; i < 4; i++) d_addr[i] = 0xff;
					send_DHCP_REQUEST(s, Cip, d_addr, pWIZNETINFO);
				break;

				case STATE_DHCP_REREQUEST :
//					jsiConsolePrintf("<<timeout>> state : STATE_DHCP_REREQUEST\r\n");
					
					for (i = 0; i < 4; i++)	d_addr[i] = DHCP_SIP[i];
					send_DHCP_REQUEST(s, (*pWIZNETINFO).ip, d_addr, pWIZNETINFO);
					
				break;
		
				default :
				break;
			}
		}
	} else {

		next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
		retry_count = 0;

		DHCP_timeout = 1;

		/* open a socket in IP RAW mode for DHCP */
		socket(s, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00);
		send_DHCP_DISCOVER(s, pWIZNETINFO);
		dhcp_state = STATE_DHCP_DISCOVER;
	}
}


// check if a leased IP is valid
int8_t check_leasedIP(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
#if 0
	iinchip_init();
	//delay 100
	Delay(10);
	
	Set_network();
	setRCR(1);

	socket(s, Sn_MR_UDP, REMOTE_CLIENT_PORT, 0x00);
	// Create UDP socket for network configuration
	//socket(SOCK_CONFIG, Sn_MR_UDP, REMOTE_CLIENT_PORT, 0x00);
	
//	jsiConsolePrintf("\r\ncheck leased IP \r\n");
	
	if (sendto(s, "CHECK_IP_CONFLICT", 17, (*pWIZNETINFO).ip, 5000) > 0)
	{
		send_DHCP_DECLINE(s);
		return 0;
	}
#endif	
	return 1;
}	

/*
*********************************************************************************************************
*              Get an IP from the DHCP server.
*
* Description: 
* Arguments  : None.
* Returns    : None.
* Note       : 
*********************************************************************************************************
*/
uint8_t getIP_DHCPS(uint8_t s, wiz_NetInfo *pWIZNETINFO)
{
	uint8_t tmpip[4];
	
	DHCP_XID = 0x12345678;

	setSHAR((*pWIZNETINFO).mac);

	// SRC IP
	tmpip[0] = 0;
	tmpip[1] = 0;
	tmpip[2] = 0;
	tmpip[3] = 0;
	setSIPR(tmpip);
	setGAR(tmpip);
	setSUBR(tmpip);
	
	//sysinit(tx_mem_conf,	rx_mem_conf);

	socket(s, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00);
	
	// Create UDP socket for network configuration
	//socket(SOCK_CONFIG, Sn_MR_UDP, REMOTE_CLIENT_PORT, 0x00);
	send_DHCP_DISCOVER(s, pWIZNETINFO);
	
	dhcp_state = STATE_DHCP_DISCOVER;
	DHCP_timeout = 0;

	next_time = jshGetSystemTime() + DHCP_WAIT_TIME;
	retry_count = 0;

	while (dhcp_state != STATE_DHCP_LEASED)
	{

  //*************
 // Add James Kim for IWatchDog
#ifdef _IWDG
		/* Reload IWDG counter */
		IWDG_ReloadCounter();
#endif	  
//
//**************
		//if (Recv_ConfigMsg() == MSG_SETTING_REQ) return(2);

		if (DHCP_timeout == 1 || jspIsInterrupted()) {
			jsiConsolePrintf("> => DHCP Timeout occurred\r\n");
			return(0);
		}
		check_DHCP_state(s, pWIZNETINFO);
		
	}
	return 1;
}

