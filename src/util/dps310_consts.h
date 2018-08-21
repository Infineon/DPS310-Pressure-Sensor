
#ifndef DPS310_CONSTS_H_
#define DPS310_CONSTS_H_


//general Constants
#define DPS310__PROD_ID						0x00
#define DPS422__PROD_ID						0x0A
#define DPS310__STD_SLAVE_ADDRESS 			0x77U
#define DPS310__SPI_WRITE_CMD 				0x00U
#define DPS310__SPI_READ_CMD 				0x80U
#define DPS310__SPI_RW_MASK 				0x80U
#define DPS310__SPI_MAX_FREQ 				100000U

// standard values
#define DPS310__TEMP_STD_MR					2U
#define DPS310__TEMP_STD_OSR				3U
#define DPS310__PRS_STD_MR					2U
#define DPS310__PRS_STD_OSR					3U
#define DPS310__OSR_SE 						3U
//we use 0.1 mS units for time calculations, so 10 units are one millisecond
#define DPS310__BUSYTIME_SCALING 			10U
// DPS310 has 10 milliseconds of spare time for each synchronous measurement / per second for asynchronous measurements
// this is for error prevention on friday-afternoon-products :D
// you can set it to 0 if you dare, but there is no warranty that it will still work
#define DPS310__BUSYTIME_FAILSAFE			10U
#define DPS310__MAX_BUSYTIME 				((1000U-DPS310__BUSYTIME_FAILSAFE)*DPS310__BUSYTIME_SCALING)

#define DPS__NUM_OF_SCAL_FACTS				8

#define DPS__SUCCEEDED						0
#define DPS__FAIL_UNKNOWN 					-1
#define DPS__FAIL_INIT_FAILED 				-2
#define DPS__FAIL_TOOBUSY 					-3
#define DPS__FAIL_UNFINISHED 				-4

#define DPS310__REG_ADR_SPI3W 				0x09U
#define DPS310__REG_CONTENT_SPI3W 			0x01U
#endif /* DPS310_CONSTS_H_ */
