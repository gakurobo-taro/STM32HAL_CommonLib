/*
 * serial_comm.cpp
 *
 *  Created on: 2023/12/26
 *      Author: yaa3k
 */

#include "serial_comm.hpp"

namespace G24_STM32HAL::CommonLib{

#ifdef USE_USB_CDC

bool UsbCdcComm::tx(uint8_t *tx_bytes,size_t size){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;

	if (cdc->TxState != 0){
		for(size_t i = 0; i < size; i++){
			tx_buff.push(tx_bytes[i]);
		}
		return true;
	}
	USBD_CDC_SetTxBuffer(usb, tx_bytes, size);
	if(USBD_CDC_TransmitPacket(usb) != USBD_OK){
		return false;
	}
	return true;
}

void UsbCdcComm::tx_interrupt_task(void){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;
	if (cdc->TxState != 0){
		size_t size = tx_buff.get_busy_level();
		uint8_t tmp_buff[64]={0};
		if(size>64) size=64;

		for(size_t i = 0; i<size; i++){
			tx_buff.pop(tmp_buff[i]);
		}
		tx(tmp_buff,size);
	}
}


size_t UsbCdcComm::rx(uint8_t *rx_bytes,size_t max_size){
	size_t avilable_data = rx_buff.get_busy_level();
	for(size_t i = 0; i < avilable_data; i++){
		if(i > max_size){
			return i;
		}
		rx_buff.pop(rx_bytes[i]);
	}
	return avilable_data;
}

void UsbCdcComm::rx_interrupt_task(const uint8_t *input,size_t size){
	for(size_t i = 0; i < size; i++){
		rx_buff.push(input[i]);
	}
}

#endif

}
