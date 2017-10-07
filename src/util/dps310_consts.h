/**
 * Arduino library to control Dps310
 *
 * "Dps310" represents Infineon's high-sensetive pressure and temperature sensor. 
 * It measures in ranges of 300 - 1200 hPa and -40 and 85 °C. 
 * The sensor can be connected via SPI or I2C. 
 * It is able to perform single measurements
 * or to perform continuous measurements of temperature and pressure at the same time, 
 * and stores the results in a FIFO to reduce bus communication. 
 *
 * Have a look at the datasheet for more information. 
 */

#ifndef DPS310_CONSTS_H_
#define DPS310_CONSTS_H_


	//general Constants
#define DPS310__PROD_ID						0U
#define DPS310__STD_SLAVE_ADDRESS 			0x77U
#define DPS310__SPI_WRITE_CMD 				0x00U
#define DPS310__SPI_READ_CMD 				0x80U
#define DPS310__SPI_RW_MASK 				0x80U
#define DPS310__SPI_MAX_FREQ 				100000U

#define DPS310__LSB 0x01U

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
#define DPS310__NUM_OF_SCAL_FACTS			8

#define DPS310__SUCCEEDED 					0
#define DPS310__FAIL_UNKNOWN 				-1
#define DPS310__FAIL_INIT_FAILED 			-2
#define DPS310__FAIL_TOOBUSY 				-3
#define DPS310__FAIL_UNFINISHED 			-4


	//Constants for register manipulation
			//SPI mode (3 or 4 wire)
#define DPS310__REG_ADR_SPI3W 				0x09U
#define DPS310__REG_CONTENT_SPI3W 			0x01U


			//product id
#define DPS310__REG_INFO_PROD_ID 			DPS310__REG_ADR_PROD_ID, \
												DPS310__REG_MASK_PROD_ID, \
												DPS310__REG_SHIFT_PROD_ID
#define DPS310__REG_ADR_PROD_ID 			0x0DU
#define DPS310__REG_MASK_PROD_ID 			0x0FU
#define DPS310__REG_SHIFT_PROD_ID 			0U

			//revision id
#define DPS310__REG_INFO_REV_ID 			DPS310__REG_ADR_REV_ID, \
												DPS310__REG_MASK_REV_ID, \
												DPS310__REG_SHIFT_REV_ID
#define DPS310__REG_ADR_REV_ID 				0x0DU
#define DPS310__REG_MASK_REV_ID 			0xF0U
#define DPS310__REG_SHIFT_REV_ID 			4U

			//operating mode
#define DPS310__REG_INFO_OPMODE 			DPS310__REG_ADR_OPMODE, \
												DPS310__REG_MASK_OPMODE, \
												DPS310__REG_SHIFT_OPMODE
#define DPS310__REG_ADR_OPMODE 				0x08U
#define DPS310__REG_MASK_OPMODE 			0x07U
#define DPS310__REG_SHIFT_OPMODE 			0U


			//temperature measure rate
#define DPS310__REG_INFO_TEMP_MR 			DPS310__REG_ADR_TEMP_MR, \
												DPS310__REG_MASK_TEMP_MR, \
												DPS310__REG_SHIFT_TEMP_MR
#define DPS310__REG_ADR_TEMP_MR 			0x07U
#define DPS310__REG_MASK_TEMP_MR 			0x70U
#define DPS310__REG_SHIFT_TEMP_MR 			4U

			//temperature oversampling rate
#define DPS310__REG_INFO_TEMP_OSR 			DPS310__REG_ADR_TEMP_OSR, \
												DPS310__REG_MASK_TEMP_OSR, \
												DPS310__REG_SHIFT_TEMP_OSR
#define DPS310__REG_ADR_TEMP_OSR 			0x07U
#define DPS310__REG_MASK_TEMP_OSR 			0x07U
#define DPS310__REG_SHIFT_TEMP_OSR 			0U

			//temperature sensor
#define DPS310__REG_INFO_TEMP_SENSOR 		DPS310__REG_ADR_TEMP_SENSOR, \
												DPS310__REG_MASK_TEMP_SENSOR, \
												DPS310__REG_SHIFT_TEMP_SENSOR
#define DPS310__REG_ADR_TEMP_SENSOR 		0x07U
#define DPS310__REG_MASK_TEMP_SENSOR 		0x80U
#define DPS310__REG_SHIFT_TEMP_SENSOR 		7U

			//temperature sensor recommendation
#define DPS310__REG_INFO_TEMP_SENSORREC 	DPS310__REG_ADR_TEMP_SENSORREC, \
												DPS310__REG_MASK_TEMP_SENSORREC, \
												DPS310__REG_SHIFT_TEMP_SENSORREC
#define DPS310__REG_ADR_TEMP_SENSORREC 		0x28U
#define DPS310__REG_MASK_TEMP_SENSORREC 	0x80U
#define DPS310__REG_SHIFT_TEMP_SENSORREC 	7U

			//temperature shift enable (if temp_osr>3)
#define DPS310__REG_INFO_TEMP_SE 			DPS310__REG_ADR_TEMP_SE, \
												DPS310__REG_MASK_TEMP_SE, \
												DPS310__REG_SHIFT_TEMP_SE
#define DPS310__REG_ADR_TEMP_SE 			0x09U
#define DPS310__REG_MASK_TEMP_SE 			0x08U
#define DPS310__REG_SHIFT_TEMP_SE 			3U


			//pressure measure rate
#define DPS310__REG_INFO_PRS_MR 			DPS310__REG_ADR_PRS_MR, \
												DPS310__REG_MASK_PRS_MR, \
												DPS310__REG_SHIFT_PRS_MR
#define DPS310__REG_ADR_PRS_MR 				0x06U
#define DPS310__REG_MASK_PRS_MR 			0x70U
#define DPS310__REG_SHIFT_PRS_MR 			4U

			//pressure oversampling rate
#define DPS310__REG_INFO_PRS_OSR 			DPS310__REG_ADR_PRS_OSR, \
												DPS310__REG_MASK_PRS_OSR, \
												DPS310__REG_SHIFT_PRS_OSR
#define DPS310__REG_ADR_PRS_OSR 			0x06U
#define DPS310__REG_MASK_PRS_OSR 			0x07U
#define DPS310__REG_SHIFT_PRS_OSR 			0U

			//pressure shift enable (if prs_osr>3)
#define DPS310__REG_INFO_PRS_SE 			DPS310__REG_ADR_PRS_SE, \
												DPS310__REG_MASK_PRS_SE, \
												DPS310__REG_SHIFT_PRS_SE
#define DPS310__REG_ADR_PRS_SE 				0x09U
#define DPS310__REG_MASK_PRS_SE 			0x04U
#define DPS310__REG_SHIFT_PRS_SE 			2U


			//temperature ready flag
#define DPS310__REG_INFO_TEMP_RDY 			DPS310__REG_ADR_TEMP_RDY, \
												DPS310__REG_MASK_TEMP_RDY, \
												DPS310__REG_SHIFT_TEMP_RDY
#define DPS310__REG_ADR_TEMP_RDY 			0x08U
#define DPS310__REG_MASK_TEMP_RDY			0x20U
#define DPS310__REG_SHIFT_TEMP_RDY 			5U

			//pressure ready flag
#define DPS310__REG_INFO_PRS_RDY 			DPS310__REG_ADR_PRS_RDY, \
												DPS310__REG_MASK_PRS_RDY, \
												DPS310__REG_SHIFT_PRS_RDY
#define DPS310__REG_ADR_PRS_RDY 			0x08U
#define DPS310__REG_MASK_PRS_RDY 			0x10U
#define DPS310__REG_SHIFT_PRS_RDY 			4U

			//pressure value
#define DPS310__REG_ADR_PRS 				0x00U
#define DPS310__REG_LEN_PRS 				3U

			//temperature value
#define DPS310__REG_ADR_TEMP 				0x03U
#define DPS310__REG_LEN_TEMP 				3U

			//compensation coefficients
#define DPS310__REG_ADR_COEF 				0x10U
#define DPS310__REG_LEN_COEF 				18


			//FIFO enable
#define DPS310__REG_INFO_FIFO_EN 			DPS310__REG_ADR_FIFO_EN, \
												DPS310__REG_MASK_FIFO_EN, \
												DPS310__REG_SHIFT_FIFO_EN
#define DPS310__REG_ADR_FIFO_EN 			0x09U
#define DPS310__REG_MASK_FIFO_EN 			0x02U
#define DPS310__REG_SHIFT_FIFO_EN 			1U

			//FIFO flush
#define DPS310__REG_INFO_FIFO_FL 			DPS310__REG_ADR_FIFO_EN, \
												DPS310__REG_MASK_FIFO_EN, \
												DPS310__REG_SHIFT_FIFO_EN
#define DPS310__REG_ADR_FIFO_FL 			0x0CU
#define DPS310__REG_MASK_FIFO_FL 			0x80U
#define DPS310__REG_SHIFT_FIFO_FL 			7U

			//FIFO empty
#define DPS310__REG_INFO_FIFO_EMPTY 		DPS310__REG_ADR_FIFO_EMPTY, \
												DPS310__REG_MASK_FIFO_EMPTY, \
												DPS310__REG_SHIFT_FIFO_EMPTY
#define DPS310__REG_ADR_FIFO_EMPTY 			0x0BU
#define DPS310__REG_MASK_FIFO_EMPTY 		0x01U
#define DPS310__REG_SHIFT_FIFO_EMPTY 		0U

			//FIFO full
#define DPS310__REG_INFO_FIFO_FULL 			DPS310__REG_ADR_FIFO_FULL, \
												DPS310__REG_MASK_FIFO_FULL, \
												DPS310__REG_SHIFT_FIFO_FULL
#define DPS310__REG_ADR_FIFO_FULL 			0x0BU
#define DPS310__REG_MASK_FIFO_FULL 			0x02U
#define DPS310__REG_SHIFT_FIFO_FULL 		1U


			//INT HL
#define DPS310__REG_INFO_INT_HL 			DPS310__REG_ADR_INT_HL, \
												DPS310__REG_MASK_INT_HL, \
												DPS310__REG_SHIFT_INT_HL
#define DPS310__REG_ADR_INT_HL 				0x09U
#define DPS310__REG_MASK_INT_HL 			0x80U
#define DPS310__REG_SHIFT_INT_HL 			7U

			//INT FIFO enable
#define DPS310__REG_INFO_INT_EN_FIFO 		DPS310__REG_ADR_INT_EN_FIFO, \
												DPS310__REG_MASK_INT_EN_FIFO, \
												DPS310__REG_SHIFT_INT_EN_FIFO
#define DPS310__REG_ADR_INT_EN_FIFO 		0x09U
#define DPS310__REG_MASK_INT_EN_FIFO 		0x40U
#define DPS310__REG_SHIFT_INT_EN_FIFO 		6U

			//INT TEMP enable
#define DPS310__REG_INFO_INT_EN_TEMP 		DPS310__REG_ADR_INT_EN_TEMP, \
												DPS310__REG_MASK_INT_EN_TEMP, \
												DPS310__REG_SHIFT_INT_EN_TEMP
#define DPS310__REG_ADR_INT_EN_TEMP 		0x09U
#define DPS310__REG_MASK_INT_EN_TEMP 		0x20U
#define DPS310__REG_SHIFT_INT_EN_TEMP 		5U

			//INT PRS enable
#define DPS310__REG_INFO_INT_EN_PRS 		DPS310__REG_ADR_INT_EN_PRS, \
												DPS310__REG_MASK_INT_EN_PRS, \
												DPS310__REG_SHIFT_INT_EN_PRS
#define DPS310__REG_ADR_INT_EN_PRS 			0x09U
#define DPS310__REG_MASK_INT_EN_PRS 		0x10U
#define DPS310__REG_SHIFT_INT_EN_PRS 		4U

			//INT FIFO flag
#define DPS310__REG_INFO_INT_FLAG_FIFO 		DPS310__REG_ADR_INT_FLAG_FIFO, \
												DPS310__REG_MASK_INT_FLAG_FIFO, \
												DPS310__REG_SHIFT_INT_FLAG_FIFO
#define DPS310__REG_ADR_INT_FLAG_FIFO 		0x0AU
#define DPS310__REG_MASK_INT_FLAG_FIFO 		0x04U
#define DPS310__REG_SHIFT_INT_FLAG_FIFO 	2U

			//INT TMP flag
#define DPS310__REG_INFO_INT_FLAG_TEMP 		DPS310__REG_ADR_INT_FLAG_TEMP, \
												DPS310__REG_MASK_INT_FLAG_TEMP, \
												DPS310__REG_SHIFT_INT_FLAG_TEMP
#define DPS310__REG_ADR_INT_FLAG_TEMP 		0x0AU
#define DPS310__REG_MASK_INT_FLAG_TEMP 		0x02U
#define DPS310__REG_SHIFT_INT_FLAG_TEMP 	1U

			//INT PRS flag
#define DPS310__REG_INFO_INT_FLAG_PRS 		DPS310__REG_ADR_INT_FLAG_PRS, \
												DPS310__REG_MASK_INT_FLAG_PRS, \
												DPS310__REG_SHIFT_INT_FLAG_PRS
#define DPS310__REG_ADR_INT_FLAG_PRS 		0x0AU
#define DPS310__REG_MASK_INT_FLAG_PRS 		0x01U
#define DPS310__REG_SHIFT_INT_FLAG_PRS 		0U



#endif /* DPS310_CONSTS_H_ */
