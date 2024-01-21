/*
 * serial_comm.cpp
 *
 *  Created on: 2023/12/26
 *      Author: yaa3k
 */

#include "serial_comm.hpp"

namespace G24_STM32HAL::CommonLib{

#ifdef USE_USB_CDC

bool UsbCdcComm::tx(const SerialData &data){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;

	if (cdc->TxState != 0){
		tx_buff.push(data);
		return true;
	}

	USBD_CDC_SetTxBuffer(usb, const_cast<uint8_t*>(data.data), data.size);
	if(USBD_CDC_TransmitPacket(usb) != USBD_OK){
		return false;
	}
	return true;
}

void UsbCdcComm::tx_interrupt_task(void){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;
	if (cdc->TxState != 0){
		SerialData tx_tmp;
		if(tx_buff.pop(tx_tmp)){
			tx(tx_tmp);
		}
	}
}

bool UsbCdcComm::rx(SerialData &data){
	return rx_buff.pop(data);
}

void UsbCdcComm::rx_interrupt_task(const uint8_t *input,size_t size){
	SerialData tmp;
	memcpy(tmp.data,input,size);
	tmp.size = size;
	rx_buff.push(tmp);
}

#endif

}
