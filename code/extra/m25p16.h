#include <avr/io.h>
#include <config.h>

/* Requires definitions of:

	RAM_CS
	RAM_PORT
	RAM_DDR


/* Register definitions for M25P16 flash ram */
#define RAM_WREN 0x06	/* write enable */
#define RAM_WRDI 0x04	/* write disable */
#define RAM_RDID 0x9F	/* read id */
#define RAM_RDSR 0x05	/* read status */
#define RAM_WRSR 0x01	/* write status */
#define RAM_READ 0x03	/* read data */
#define RAM_FASTREAD 0x0B	
#define RAM_PP 0x02		/* page program */
#define RAM_SE 0xD8		/* sector erase */
#define RAM_BE 0xC7		/* bulk erase */
#define RAM_DP 0xB9		/* deep power down */
#define RAM_RES 0xAB	/* release from power down, read electronic signature */


/** \brief		Read 3 bytes of ID from the M26P16. These should always be 0x20,0x20,0x15
 *	\param		*mem_ptr	    Pointer to location to store values returned.
 *
 */
void read_ram_id(uint8_t* mem_ptr);
 
 
 
/** \brief		Select the M25P16 and return 1 byte from the Status register.
 *	\return					returns device status byte.
 *
 */ 
uint8_t read_ram_status(void);



/** \brief		Erase the M25P16.
 *
 *				This function issues an erase command, then blocks until the command is complete as shown by
 *				 the status register being zero.
 *				Note that the erase actually sets all bits to 1. The page program can set bits to 0, but NOT to 1.
 *				Therefore each page should be considered 'write once' between erase cycles.
 */
void ram_bulk_erase(void);



/** \brief		Erase 1 sector of the M25P16.
 *	\param		sector	    The sector number (0-31) to be erased.
 *
 *				This function issues a sector erase command, then blocks until the command is complete as shown by
 *				 the status register being zero
 *				For future expansion, no masking of the sector byte to ensure that it does not contain values >31 takes place
 *				Note that the erase actually sets all bits to 1. The page program can set bits to 0, but NOT to 1.
 *				Therefore each page should be considered 'write once' between erase cycles.
 */
void ram_sector_erase(uint8_t sector);


/** \brief		Write exactly 256 bytes to the selected page of the M26P16 from memory
 *	\param		flash_sector	Sector number in the M25P16, range 0..31.
 *	\param		flash_page		Page number in the sector.
 *	\param		*mem_ptr		Pointer to the data location in memory
 *
 *				The 256 bytes will be exactly aligned with a 256 byte boundary in the memory
 *				Writing 256 bytes at a time is the most efficient way to write to this device
 *
 *				For reads or writes of less than 256 bytes, or non aligned read or writes use the read_write_flash_ram() function instead.
 */
void write_flash_page(uint8_t flash_sector,uint8_t flash_page,uint8_t* mem_ptr);



/** \brief		Read exactly 256 bytes from the selected page of the M26P16 to memory
 *	\param		flash_sector	Sector number in the M25P16, range 0..31.
 *	\param		flash_page		Page number in the sector.
 *	\param		*mem_ptr		Pointer to the data location in memory
 *
 *				The function will carry out the write enable instruction as part of the write
 *				The 256 bytes will be exactly aligned with a 256 byte boundary in the memory
 *
 *				For reads or writes of less than 256 bytes, or non aligned read or writes use the read_write_flash_ram() function instead.
 */
void read_flash_page(uint8_t flash_sector,uint8_t flash_page,uint8_t* mem_ptr);



/** \brief		Perform an arbirary read/write from/to the M26P16
 *	\param		one_read_zero_write		Any non-zero value will execute a read from the M25P16, 0 will execute a write to the M25P16
 *	\param		bytes_to_readwrite		uint16_t Number of bytes to read or write. Values will typically be less than or exactly 256. See below for why.
 *	\param		flash_sector			Sector number in the M25P16, range 0..31.
 *	\param		flash_page				Page number in the sector.
 *	\param		offset					Position in the page at which to start the read/write
 *	\param		*mem_ptr				Pointer to memory location for read/write data
 *
 *				This function would typically only be used for reads or writes of <256 bytes.
 *
 *				IMPORTANT   	The M25P16 is a block device. It deals in 256 byte pages.
 *								Writes only every take place to a single 256 byte page.
 *								If writing >256 bytes, anything other than the last 256 bytes will be overwritten and ignored.
 *								If offset is non-zero, then be aware that if offset+bytes_to_readwrite > 255, then any write
 *								  will wrap back to the beginning of the page. This is unlikely to be what you want.
 */
void read_write_flash_ram(uint8_t one_read_zero_write,uint16_t bytes_to_readwrite,uint8_t flash_sector,uint8_t flash_page,uint8_t offset,uint8_t* mem_ptr);



/** \brief		Write to the status register on the M25P16.
 *	\param		status		Value to write
 *
 *				This function blocks until the bottom bit of the status register is clear = device ready.
 */
void write_ram_status(uint8_t status);



/** \brief		Issue the command to bring the M25P16 out of power down mode.
 *
 *				This function has no effect if the device is currently in one of the erase modes.
 *				At power up the deice will be in standby mode, there is no need to issue the power_up_flash_ram() command after a power up.
 */
void power_up_flash_ram(void);



/** \brief		Issue the command to put the M25P16 into power down mode.
 *
 *				In Power down mode the device ignores all erase and program instructions.
 *
 *				In this mode the device draws 1uA typically. 
 *				Use the power_up_flash_ram() command to bring the device out of power down mode.
 *				Removing power completely will also cancel the Deep power down mode - it will power up again in standby mode.
 */
void power_down_flash_ram(void);

