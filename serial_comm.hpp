/*
 * serial_comm.hpp
 *
 *  Created on: 2023/12/26
 *      Author: yaa3k
 */

#ifndef SERIAL_COMM_HPP_
#define SERIAL_COMM_HPP_

#include "main.h"
#include "ring_buffer.hpp"
#ifdef USE_USB_CDC
#include "usbd_cdc_if.h"
#include "usb_device.h"
#endif

namespace G24_STM32HAL::CommonLib{

struct SerialData{
	static constexpr size_t max_size = 64;
	uint8_t data[max_size] = {0};
	size_t size = 0;
};

class ISerial{
public:
	virtual bool tx(const SerialData &data) = 0;
	virtual size_t tx_available(void) = 0;

	virtual bool rx(SerialData &data) = 0;
	virtual size_t rx_available(void) = 0;
};

#ifdef USE_USB_CDC

template<size_t TX_BUFF_N,size_t RX_BUFF_N>
class UsbCdcComm : ISerial{
private:
	USBD_HandleTypeDef *usb;
	RingBuffer<SerialData,TX_BUFF_N> tx_buff;
	RingBuffer<SerialData,RX_BUFF_N> rx_buff;

public:
	UsbCdcComm(USBD_HandleTypeDef *_usb):usb(_usb){}

	USBD_HandleTypeDef *get_usb_handle(void)const{
		return usb;
	}

	//tx functions
	bool tx(const SerialData &data) override;
	size_t tx_available(void) override{
		return tx_buff.get_free_level();
	}
	void tx_interrupt_task(void);

	//rx functions
	bool rx(SerialData &data) override{
		return rx_buff.pop(data);
	}
	size_t rx_available(void) override{
		return rx_buff.get_busy_level();
	}
	void rx_interrupt_task(const uint8_t *input,size_t size);

};

//////////////////////////////////////////////////////////////////////////
//CanComm function references
//////////////////////////////////////////////////////////////////////////

template<size_t TX_BUFF_N,size_t RX_BUFF_N>
bool UsbCdcComm<TX_BUFF_N,RX_BUFF_N>::tx(const SerialData &data){
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

template<size_t TX_BUFF_N,size_t RX_BUFF_N>
void UsbCdcComm<TX_BUFF_N,RX_BUFF_N>::tx_interrupt_task(void){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;
	if (cdc->TxState != 0){
		SerialData tx_tmp;
		if(tx_buff.pop(tx_tmp)){
			tx(tx_tmp);
		}
	}
}

template<size_t TX_BUFF_N,size_t RX_BUFF_N>
void UsbCdcComm<TX_BUFF_N,RX_BUFF_N>::rx_interrupt_task(const uint8_t *input,size_t size){
	SerialData tmp;
	memcpy(tmp.data,input,size);
	tmp.size = size;
	rx_buff.push(tmp);
}


#endif

#ifdef USE_UART
class UsbCdcComm : ISerial{

};
#endif

}
#endif /* SERIAL_COMM_HPP_ */
