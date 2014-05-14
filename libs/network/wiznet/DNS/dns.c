#include "jsutils.h"
#include "jshardware.h"


#include "dns.h"
#include "wiznet/Ethernet/socket.h"
#include "wiznet/DNS/dns_parse.h"


extern struct _CHCONF ChConf;
/*
********************************************************************************
Define Part
********************************************************************************
*/
//#define DBG_DNS

/*
********************************************************************************
Local Variable Declaration Section
********************************************************************************
*/
uint16_t MSG_ID = 0x1122;

/*
********************************************************************************
Function Implementation Part
********************************************************************************
*/

/*
********************************************************************************
*              PUT NETWORK BYTE ORDERED INT.
*
* Description : This function copies uint16_t to the network buffer with network byte order.
* Arguments   : s - is a pointer to the network buffer.
*               i - is a unsigned integer.
* Returns     : a pointer to the buffer.
* Note        : Internal Function
********************************************************************************
*/
uint8_t * put16(uint8_t * s, uint16_t i)
{
	*s++ = i >> 8;
	*s++ = i;

	return s;
}


/*
********************************************************************************
*              MAKE DNS QUERY MESSAGE
*
* Description : This function makes DNS query message.
* Arguments   : op   - Recursion desired
*               name - is a pointer to the domain name.
*               buf  - is a pointer to the buffer for DNS message.
*               len  - is the MAX. size of buffer.
* Returns     : the pointer to the DNS message.
* Note        :
********************************************************************************
*/
//int16_t dns_makequery(uint16_t op, uint8_t * name, uint8_t * buf, uint16_t len)
int16_t dns_makequery(uint16_t op, char * name, uint8_t * buf, uint16_t len)
{
	uint8_t *cp;
	char *cp1;
	char sname[MAX_DNS_BUF_SIZE];
	char *dname;
	uint16_t p;
	uint16_t dlen;

	cp = buf;

	MSG_ID++;
	cp = put16(cp, MSG_ID);
	p = (op << 11) | 0x0100;			/* Recursion desired */
	cp = put16(cp, p);
	cp = put16(cp, 1);
	cp = put16(cp, 0);
	cp = put16(cp, 0);
	cp = put16(cp, 0);

	strncpy(sname, name, MAX_DNS_BUF_SIZE);
	dname = sname;
	dlen = strlen(dname);
	for (;;)
	{
#ifdef IWDG
		/* Reload IWDG counter */
		IWDG_ReloadCounter();

#endif	  
		/* Look for next dot */
		cp1 = dname;
		while (*cp1 && *cp1!='.') cp1++;
		if (!*cp1) cp1 = 0;

		if (cp1 != 0) len = cp1 - dname;	/* More to come */
		else len = dlen;			/* Last component */

		*cp++ = len;				/* Write length of component */
		if (len == 0) break;

		/* Copy component up to (but not including) dot */
		strncpy((char *)cp, dname, len);
		cp += len;
		if (cp1 == 0)
		{
			*cp++ = 0;			/* Last one; write null and finish */
			break;
		}
		dname += len+1;
		dlen -= len+1;
	}

	cp = put16(cp, 0x0001);				/* type */
	cp = put16(cp, 0x0001);				/* class */

//	return ((int16_t)((uint16_t)(cp) - (uint16_t)(buf)));
	return ((int16_t)((uint32_t)(cp) - (uint32_t)(buf)));
}

/*
********************************************************************************
*              MAKE DNS QUERY AND PARSE THE REPLY
*
* Description : This function makes DNS query message and parses the reply from DNS server.
* Arguments   : name - is a pointer to the domain name.
* Returns     : if succeeds : 1, fails : -1
* Note        :
********************************************************************************
*/
uint8_t dns_query(uint8_t ch, uint8_t s, uint8_t * name)
{
	struct dhdr dhp;
	uint8_t ip[4];
	uint16_t len, port;
	uint16_t cnt;

	wiz_NetInfo gWIZNETINFO;
	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
	uint8_t dns_buf[MAX_DNS_BUF_SIZE];

	for(;socket(s, Sn_MR_UDP, (rand() & 32767) + 2000, 0) != s;)
	{
	}
	len = dns_makequery(0, (char *)name, dns_buf, MAX_DNS_BUF_SIZE);
	cnt = sendto(s, dns_buf, len, gWIZNETINFO.dns, IPPORT_DOMAIN);

	JsSysTime endTime = jshGetSystemTime() + jshGetTimeFromMilliseconds(1000);

	while (1)
	{
		if ((len = getSn_RX_RSR(s)) > 0)
		{
			if (len > MAX_DNS_BUF_SIZE) len = MAX_DNS_BUF_SIZE;

			len = recvfrom(s, dns_buf, len, ip, &port);

			//close(s);
			break;
		}
		//wait_1ms(1);
		jshDelayMicroseconds(1000);

		if (jshGetSystemTime() > endTime)
		{
			return 0;
		}
	}

	//mgg1010 - next line added to ensure socket is closed - without this, it runs out of sockets
	
	close(s);
	
//	for(i=0;i<256;i++) PrintString("%02bx ", dns_buf[i]);
	return(parseMSG(ch, &dhp, dns_buf));	/* Convert to local format */
}
