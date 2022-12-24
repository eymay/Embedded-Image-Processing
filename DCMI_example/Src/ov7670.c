#include "ov7670.h"

//#define DEBUG

I2C_HandleTypeDef *phi2c;
DCMI_HandleTypeDef *phdcmi;
UART_HandleTypeDef *phuart;

void OV2640_Init(I2C_HandleTypeDef *p_hi2c, DCMI_HandleTypeDef *p_hdcmi) {
	phi2c = p_hi2c;
	phdcmi = p_hdcmi;

	// Hardware reset
	HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(CAMERA_RESET_GPIO_Port, CAMERA_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	// Software reset: reset all registers to default values
	SCCB_Write(0xff, 0x01);
	SCCB_Write(0x12, 0x80);
	HAL_Delay(100);

#ifdef DEBUG
	uint8_t pid;
	uint8_t ver;
	SCCB_Read(0x0a, &pid);  // pid value is 0x26
	SCCB_Read(0x0b, &ver);  // ver value is 0x42
	my_printf("PID: 0x%x, VER: 0x%x\n", pid, ver);
#endif

	// Stop DCMI clear buffer
	OV2640_StopDCMI();
}

void OV2640_ResolutionOptions(uint16_t opt) {
	switch (opt) {
	case 15533:
		OV2640_ResolutionConfiguration(0);
		break;
	case 15534:
		OV2640_ResolutionConfiguration(1);
		break;
	case 15535:
		OV2640_ResolutionConfiguration(2);
		break;
	case 25535:
		OV2640_ResolutionConfiguration(3);
		break;
	case 45535:
		OV2640_ResolutionConfiguration(4);
		break;
	case 65535:
		OV2640_ResolutionConfiguration(5);
		break;
	default:
		OV2640_ResolutionConfiguration(1);
		break;
	}

}

void OV2640_ResolutionConfiguration(short opt) {
#ifdef DEBUG
	my_printf("Starting resolution choice \r\n");
#endif
	OV2640_Configuration(OV2640_JPEG_INIT);
	OV2640_Configuration(OV2640_YUV422);
	OV2640_Configuration(OV2640_JPEG);
	HAL_Delay(10);
	SCCB_Write(0xff, 0x01);
	HAL_Delay(10);
	SCCB_Write(0x15, 0x00);

	switch (opt) {
	case 0:
		OV2640_Configuration(OV2640_160x120_JPEG);
		break;
	case 1:
		OV2640_Configuration(OV2640_320x240_JPEG);
		break;
	case 2:
		OV2640_Configuration(OV2640_640x480_JPEG);
		break;
	case 3:
		OV2640_Configuration(OV2640_800x600_JPEG);
		break;
	case 4:
		OV2640_Configuration(OV2640_1024x768_JPEG);
		break;
	case 5:
		OV2640_Configuration(OV2640_1280x960_JPEG);
		break;
	default:
		OV2640_Configuration(OV2640_320x240_JPEG);
		break;
	}

#ifdef DEBUG
	my_printf("Finalize configuration \r\n");
#endif
}

void OV2640_Configuration(const unsigned char arr[][2]) {
	unsigned short i = 0;
	uint8_t reg_addr, data, data_read;
	while (1) {
		reg_addr = arr[i][0];
		data = arr[i][1];
		if (reg_addr == 0xff && data == 0xff) {
			break;
		}
		SCCB_Read(reg_addr, &data_read);
		SCCB_Write(reg_addr, data);
#ifdef DEBUG
		my_printf("SCCB write: 0x%x 0x%x=>0x%x\r\n", reg_addr, data_read, data);
#endif
		HAL_Delay(10);
		SCCB_Read(reg_addr, &data_read);
		if (data != data_read) {
#ifdef DEBUG
			my_printf("SCCB write failure: 0x%x 0x%x\r\n", reg_addr, data_read);
#endif
		}
		i++;
	}
}

/**
 * Stop DCMI (Clear  memory buffer)
 */
void OV2640_StopDCMI(void) {
#ifdef DEBUG
	my_printf("DCMI has been stopped \r\n");
#endif
	HAL_DCMI_Stop(phdcmi);
	HAL_Delay(10); // If you get a DCMI error (data is not received), increase value to 30.
}

/**
 * Executes a single reading from DCMI and returns  data as an image.
 * @param frameBuffer Table with data.
 * @param length Length of capture to be transferred.
 */
void OV2640_CaptureSnapshot(uint32_t frameBuffer, int length) {
	HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_SNAPSHOT, frameBuffer, length);
	HAL_Delay(2000);
	HAL_DCMI_Suspend(phdcmi);
	HAL_DCMI_Stop(phdcmi);
}

/**
 * Write value to camera register.
 * @param reg_addr Address of register.
 * @param data New value.
 * @return  Operation status.
 */
short SCCB_Write(uint8_t reg_addr, uint8_t data) {
	short opertionStatus = 0;
	uint8_t buffer[2] = { 0 };
	HAL_StatusTypeDef connectionStatus;
	buffer[0] = reg_addr;
	buffer[1] = data;
	__disable_irq();
	connectionStatus = HAL_I2C_Master_Transmit(phi2c, (uint16_t) 0x60, buffer,
			2, 100);
	if (connectionStatus == HAL_OK) {
		opertionStatus = 1;
	} else {
		opertionStatus = 0;
	}
	__enable_irq();
	return opertionStatus;
}

/**
 * Reading data from camera registers.
 * @param reg_addr Address of register.
 * @param pdata Value read from register.
 * @return Operation status.
 */
short SCCB_Read(uint8_t reg_addr, uint8_t *pdata) {
	short opertionStatus = 0;
	HAL_StatusTypeDef connectionStatus;
	__disable_irq();
	connectionStatus = HAL_I2C_Master_Transmit(phi2c, (uint16_t) 0x60,
			&reg_addr, 1, 100);
	if (connectionStatus == HAL_OK) {
		connectionStatus = HAL_I2C_Master_Receive(phi2c, (uint16_t) 0x61, pdata,
				1, 100);
		if (connectionStatus == HAL_OK) {
			opertionStatus = 0;
		} else {
			opertionStatus = 1;
		}
	} else {
		opertionStatus = 2;
	}
	__enable_irq();
	return opertionStatus;
}
