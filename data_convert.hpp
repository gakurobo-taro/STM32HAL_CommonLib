/*
 * data_convert.hpp
 *
 *  Created on: 2023/12/25
 *      Author: yaa3k
 */

#ifndef DATA_CONVERT_HPP_
#define DATA_CONVERT_HPP_

#include <stdio.h>
#include <string.h>
#include "can_comm.hpp"
#include "data_packet.hpp"

namespace G24_STM32HAL::CommonLib{

namespace DataConvert{
	constexpr size_t PRIORITY_BIT = 24;
	constexpr size_t DATA_TYPE_BIT = 20;
	constexpr size_t BOARD_ID_BIT = 16;
	constexpr size_t REGISTER_ID_BIT = 0;

	//encode/decode can_frame_t
	bool encode_can_frame(const DataPacket &data,CanFrame &can_frame);
	bool decode_can_frame(const CanFrame &can_frame,DataPacket &data);

	//encode/decode Cobsbytes
	size_t encode_bytes(const DataPacket &data,uint8_t *output,size_t max_size);
	size_t encode_COBS_bytes(const DataPacket &data,uint8_t *output,size_t max_size);
	bool decode_bytes(const uint8_t *input,const size_t input_size,DataPacket &data);
	bool decode_COBS_bytes(const uint8_t *input,DataPacket &data);

	//encode/decode slcan str
	constexpr size_t SLCAN_STR_MAX_SIZE = 28;
	size_t can_to_slcan(const CanFrame &frame,char *str,const size_t str_max_size);
	bool slcan_to_can(const char *str, CanFrame &frame);

	size_t encode_COBS(const uint8_t *input,size_t input_size,uint8_t *output,size_t output_size_limit);
	size_t decode_COBS(const uint8_t *input,uint8_t *output, size_t output_size_limit);
}
}

#endif /* DATA_CONVERT_HPP_ */
