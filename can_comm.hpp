/*
 * CAN_communication.hpp
 *
 *  Created on: 2023/12/21
 *      Author: yaa3k
 */

#ifndef CAN_COMM_HPP_
#define CAN_COMM_HPP_

#include "main.h"
#include "ring_buffer.hpp"
#include "gpio.h"

namespace G24_STM32HAL::CommonLib{

struct CanFrame{
	uint8_t data[8]={0};
	size_t size=0;
	uint32_t id=0;
	bool is_ext_id=false;
	bool is_remote=false;
};

enum class FilterMode{
	ONLY_STD,
	ONLY_EXT,
	STD_AND_EXT,
};

#ifdef USE_CAN
class CanComm{
private:
	CAN_HandleTypeDef *can;
	const uint32_t rx_fifo;
	const uint32_t rx_filter_fifo;
	const uint32_t rx_fifo_it;

	RingBuffer<CanFrame, BuffSize::SIZE16> rx_buff;

public:
	CanComm(CAN_HandleTypeDef *_can,uint32_t _rx_fifo,uint32_t _rx_filter_fifo,uint32_t _rx_fifo_it)
	:can(_can),rx_fifo(_rx_fifo),rx_filter_fifo(_rx_filter_fifo),rx_fifo_it(_rx_fifo_it){
	}

	void start(void);

	//can tx functions/////////////////////////////
	bool tx(CanFrame &tx_data);
	//inline functions
	uint32_t tx_available(void){
		return HAL_CAN_GetTxMailboxesFreeLevel(can);
	}

	//can rx fuctions//////////////////////////////
	uint32_t rx_available(void);
	void rx_interrupt_task(void);
	bool rx(CanFrame &rx_frame);

	//can filter setting///////////////////////////
	void set_filter_mask(uint32_t filter_no,uint32_t id,uint32_t mask,FilterMode mode,bool as_std);
	void set_filter_free(uint32_t filter_no);
};
#endif

}

#endif /* CAN_COMM_HPP_ */
