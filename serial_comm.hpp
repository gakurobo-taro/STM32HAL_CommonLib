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

class UsbCdcComm : ISerial{
private:
	USBD_HandleTypeDef *usb;
	RingBuffer<SerialData,(size_t)BuffSize::SIZE8> tx_buff;
	RingBuffer<SerialData,(size_t)BuffSize::SIZE8> rx_buff;

public:
	UsbCdcComm(USBD_HandleTypeDef *_usb):usb(_usb){}

	//tx functions
	bool tx(const SerialData &data) override;
	size_t tx_available(void) override{
		return tx_buff.get_free_level();
	}
	void tx_interrupt_task(void);

	//rx functions
	bool rx(SerialData &data) override;
	size_t rx_available(void) override{
		return rx_buff.get_busy_level();
	}
	void rx_interrupt_task(const uint8_t *input,size_t size);

};


#endif

#ifdef USE_UART
class UsbCdcComm : ISerial{

};
#endif

}
#endif /* SERIAL_COMM_HPP_ */
