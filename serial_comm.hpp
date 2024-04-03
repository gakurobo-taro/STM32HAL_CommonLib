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
#include <memory>

#ifdef USE_USB_CDC
#include "usbd_cdc_if.h"
#include "usb_device.h"
#endif

#ifdef USE_UART
#include "usart.h"
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
	virtual size_t tx_available(void) const = 0;

	virtual bool rx(SerialData &data) = 0;
	virtual size_t rx_available(void) const = 0;
};

#ifdef USE_USB_CDC

class UsbCdcComm : ISerial{
private:
	USBD_HandleTypeDef *usb;
	std::unique_ptr<IRingBuffer<SerialData>> rx_buff;
	std::unique_ptr<IRingBuffer<SerialData>> tx_buff;

	SerialData tmp_buff;

public:
	UsbCdcComm(USBD_HandleTypeDef *_usb,std::unique_ptr<IRingBuffer<SerialData>> _rx_buff,std::unique_ptr<IRingBuffer<SerialData>> &&_tx_buff)
	:usb(_usb),
	 rx_buff(std::move(_rx_buff)),
	 tx_buff(std::move(_tx_buff)){
	}

	USBD_HandleTypeDef *get_usb_handle(void)const{
		return usb;
	}

	//tx functions
	bool tx(const SerialData &data) override;
	size_t tx_available(void)const override{
		return tx_buff->get_free_level();
	}
	void tx_interrupt_task(void);

	//rx functions
	bool rx(SerialData &data) override{
		return rx_buff->pop(data);
	}
	size_t rx_available(void) const override{
		return rx_buff->get_busy_level();
	}
	void rx_interrupt_task(const uint8_t *input,size_t size);

};

//////////////////////////////////////////////////////////////////////////
//CanComm function references
//////////////////////////////////////////////////////////////////////////
inline bool UsbCdcComm::tx(const SerialData &data){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;

	if (cdc->TxState != 0){
		tx_buff->push(data);
		return true;
	}

	USBD_CDC_SetTxBuffer(usb, const_cast<uint8_t*>(data.data), data.size);
	if(USBD_CDC_TransmitPacket(usb) != USBD_OK){
		return false;
	}
	return true;
}

inline void UsbCdcComm::tx_interrupt_task(void){
	USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef*)usb->pClassData;
	if (cdc->TxState != 0){
		SerialData tx_tmp;
		if(tx_buff->pop(tx_tmp)){
			tx(tx_tmp);
		}
	}
}

inline void UsbCdcComm::rx_interrupt_task(const uint8_t *input,size_t size){
	for(size_t i = 0; i < size; i++){
		if((input[i]=='\r') || (input[i]=='\n') || (input[i]=='\0') || (tmp_buff.size >= tmp_buff.max_size-1)){
			tmp_buff.data[tmp_buff.size] = input[i];
			tmp_buff.size ++;

			rx_buff->push(tmp_buff);

			tmp_buff.size = 0;
			memset(tmp_buff.data,0,tmp_buff.max_size);
		}else{
			tmp_buff.data[tmp_buff.size] = input[i];
			tmp_buff.size ++;
		}
	}

}


#endif

#ifdef USE_UART
class UartComm : ISerial{
private:
	UART_HandleTypeDef* uart;
	std::unique_ptr<IRingBuffer<SerialData>> rx_buff;
	std::unique_ptr<IRingBuffer<SerialData>> tx_buff;

	uint8_t rx_tmp_byte;
	SerialData rx_tmp_packet;

	SerialData tx_tmp_packet;
	bool is_transmitting = false;
public:
	UartComm(UART_HandleTypeDef *_uart,std::unique_ptr<IRingBuffer<SerialData>> _rx_buff,std::unique_ptr<IRingBuffer<SerialData>> _tx_buff):
		uart(_uart),
		rx_buff(std::move(_rx_buff)),
		tx_buff(std::move(_tx_buff)){
	}

	UART_HandleTypeDef *get_handle(void)const{
		return uart;
	}

	//tx functions
	bool tx(const SerialData &data) override{
		if(is_transmitting){
			tx_buff->push(data);
		}else{
			tx_tmp_packet = data;
			HAL_UART_Transmit_IT(uart, const_cast<uint8_t*>(tx_tmp_packet.data), tx_tmp_packet.size);
			is_transmitting = true;
			return true;
		}
	}
	size_t tx_available(void)const override{
		return tx_buff->get_free_level();
	}
	void tx_interrupt_task(void){
		if(tx_buff->get_busy_level()){
			tx_buff->pop(tx_tmp_packet);
			HAL_UART_Transmit_IT(uart, const_cast<uint8_t*>(tx_tmp_packet.data), tx_tmp_packet.size);
			is_transmitting = true;
		}else{
			is_transmitting = false;
		}
	}

	//rx functions
	void rx_start(void){
		HAL_UART_Receive_IT(uart, &rx_tmp_byte, 1);
	}
	bool rx(SerialData &data) override{
		return rx_buff->pop(data);
	}
	size_t rx_available(void) const override{
		return rx_buff->get_busy_level();
	}

	void rx_interrupt_task(void){
		if((rx_tmp_byte=='\r') || (rx_tmp_byte=='\n') || (rx_tmp_byte=='\0') || (rx_tmp_packet.size >= rx_tmp_packet.max_size-1)){
			rx_tmp_packet.data[rx_tmp_packet.size] = rx_tmp_byte;
			rx_tmp_packet.size ++;

			rx_buff->push(rx_tmp_packet);

			rx_tmp_packet = SerialData{};
		}else{
			rx_tmp_packet.data[rx_tmp_packet.size] = rx_tmp_byte;
			rx_tmp_packet.size ++;
		}

		rx_start();
	}
};
#endif

}
#endif /* SERIAL_COMM_HPP_ */
