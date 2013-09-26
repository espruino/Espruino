#include <avr/io.h>
#include <config.h>
#include <m25p16.h>
#include <spi2.h>
#include <util/delay_basic.h>
#include <util/delay.h>

/* Support for M25P16 2Mbyte flash RAM 


BEWARE  Programming can only set bits to zero. IT CANNOT CHANGE A ZERO TO A ONE
You must use the Bulk or sector erase to set bits back to a one


	RAM_PORT		PORT for the RAM CS signal
	RAM_CS


and RAM registers

	RAM_RDID
	RAM_WREN
	RAM_BE
	RAM_PP
	RAM_RDSR
	
and of functions:

	xmit_spi
	recv_spi

Additionally, RAM_CS must be defined as an output on the appropriate DDR

*/


void read_ram_id(uint8_t* mem_ptr) {

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_RDID);
	mem_ptr[0] = recv_spi();
	mem_ptr[1] = recv_spi();
	mem_ptr[2] = recv_spi();
	RAM_PORT |= 1<<RAM_CS;				// deassert cs 
}



uint8_t read_ram_status(void) {

	uint8_t status;

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_RDSR);
	status = recv_spi();
	RAM_PORT |= 1<<RAM_CS;				// deassert cs 
	return status;
}


void ram_bulk_erase(void) {
		
	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_WREN);					// write enable instruction 	
	RAM_PORT |= (1<<RAM_CS);			// deassert cs 
	_delay_us(1); 
	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_BE);					// bulk erase instruction 
	RAM_PORT |= (1<<RAM_CS);			// deassert cs 
	while (read_ram_status())
		;
}


void ram_sector_erase(uint8_t sector) {
		

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_WREN);					// write enable instruction 	
	RAM_PORT |= (1<<RAM_CS);			// deassert cs 
	_delay_us(1); 
	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_SE);					// sector erase instruction 
	xmit_spi(sector);					// sector erase instruction 
	xmit_spi(0x00);						// sector erase instruction 
	xmit_spi(0x00);						// sector erase instruction 
	RAM_PORT |= (1<<RAM_CS);			// deassert cs 
	while (read_ram_status())
		;
}

void write_flash_page(uint8_t flash_sector,uint8_t flash_page,uint8_t* mem_ptr) {
	read_write_flash_ram(0,256,flash_sector,flash_page,0,mem_ptr);
}
void read_flash_page(uint8_t flash_sector,uint8_t flash_page,uint8_t* mem_ptr) {
	read_write_flash_ram(1,256,flash_sector,flash_page,0,mem_ptr);
}



void read_write_flash_ram(uint8_t one_read_zero_write,uint16_t bytes_to_readwrite,uint8_t flash_sector,uint8_t flash_page,uint8_t offset,uint8_t* mem_ptr) {

// NB CAUTION page writes which cross page boundaries will wrap 


// parameters 

// one_read_zero_write = 1 for read, 0 for write 
// bytes_to_readwrite to read or write 
// flash sector within device 
// flash page within device 
// offset for first byte to transfer 
// POINTER TO ram address for first byte to transfer 


	uint16_t i;

// for ram device, enter and leave with SCK low 


	RAM_PORT &= ~(1<<RAM_CS);				// assert cs 
	if (one_read_zero_write) {
		xmit_spi(RAM_READ);
	} else {
		xmit_spi(RAM_WREN);				// write enable instruction 
		RAM_PORT |= (1<<RAM_CS);
		_delay_us(1);
		RAM_PORT &= ~(1<<RAM_CS);
		xmit_spi(RAM_PP);
	}
	xmit_spi(flash_sector);
	xmit_spi(flash_page);
	xmit_spi(offset);
	for (i=0;i<bytes_to_readwrite;i++) {
		if (one_read_zero_write) {
			mem_ptr[i] = recv_spi();
		} else {
			xmit_spi(mem_ptr[i]);
		}
	}	
	RAM_PORT |= (1<<RAM_CS);

	_delay_ms(400);
	while (read_ram_status()) {
		_delay_ms(10); 
	}	
}


// write to the RAM status byte. 0 in bottom bit position = ready 
void write_ram_status(uint8_t status) {

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_WREN);					// write enable instruction 	
	RAM_PORT |= (1<<RAM_CS);			// deassert cs 
	_delay_us(2); 
	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_WRSR);
	xmit_spi(status);
	RAM_PORT |= 1<<RAM_CS;				// deassert cs 
	_delay_us(2);
	while (read_ram_status() & 0x01) 
		;	
}


void power_up_flash_ram(void) {

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_RES);
	RAM_PORT |= 1<<RAM_CS;				// deassert cs 
	_delay_us(30);
}


void power_down_flash_ram(void) {

	RAM_PORT &= ~(1<<RAM_CS);			// assert cs 
	xmit_spi(RAM_DP);
	RAM_PORT |= 1<<RAM_CS;				// deassert cs 
}

