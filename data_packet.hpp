/*
 * data_packet.hpp
 *
 *  Created on: Dec 23, 2023
 *      Author: yaa3k
 */

#ifndef DATA_PACKET_HPP_
#define DATA_PACKET_HPP_

#include <stdint.h>

namespace G24_STM32HAL::CommonLib{

enum class DataType : uint8_t{
	COMMON_DATA,
	PCU_DATA,
	RMC_DAT,
	GPIOC_DATA,
	COMMON_DATA_ENFORCE = 0xFF
};

struct DataPacket{
	bool is_request = false;
	uint8_t priority = 7;
	DataType data_type = DataType::COMMON_DATA;
	uint8_t board_ID = 0;
	uint16_t register_ID = 0;

	uint8_t data[8] = {0};
	size_t data_length = 0;

	void set_float_data(float fdata){
		*reinterpret_cast<float*>(data) = fdata;
		data_length = sizeof(float)/sizeof(uint8_t);
	}
	float get_float_data(void){
		return *reinterpret_cast<float*>(data);
	}
};

}

#endif /* DATA_PACKET_HPP_ */
