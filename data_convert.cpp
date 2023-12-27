/*
 * data_packet.cpp
 *
 *  Created on: Dec 23, 2023
 *      Author: yaa3k
 */


#include "data_convert.hpp"

namespace G24_STM32HAL::CommonLib{
namespace DataConvert{

bool encode_can_frame(const DataPacket &data,CanFrame &can_frame){

	can_frame.is_ext_id = true;
	can_frame.is_remote = data.is_request;

	can_frame.id = ((data.priority&0x7)<<26) | (((uint8_t)data.data_type&0xF)<<22)
			| ((data.board_ID&0xF)<<18) | (data.register_ID&0xFFFF);

	can_frame.size = data.data_length;
	for(size_t i = 0; i < data.data_length; i++){
		can_frame.data[i] = data.data[i];
	}
	return true;

}
bool decode_can_frame(const CanFrame &can_frame,DataPacket &data){
	if(can_frame.is_ext_id){
		data.is_request = can_frame.is_remote;
		data.priority = (can_frame.id>>26)&0x7;
		data.data_type = (DataType)((can_frame.id >> 22)&0xF);
		data.board_ID = (can_frame.id >> 18)&0xF;
		data.register_ID = can_frame.id & 0xFFFF;

		for(size_t i = 0; i<can_frame.size; i++){
			data.data[i] = can_frame.data[i];
		}
		data.data_length = can_frame.size;

		return true;
	}else{
		return false;
	}

}

size_t encode_bytes(const DataPacket &data,uint8_t *output,size_t max_size){
	if(max_size >= 4){
		output[0] = data.priority&0x7;
		output[1] = (((uint8_t)data.data_type&0xF) << 4)|(data.board_ID&0xF);
		output[2] = (data.register_ID>>8)&0xFF;
		output[3] = data.register_ID & 0xFF;
	}
	for(size_t i = 4; i < (data.data_length+4); i++){
		output[i] = data.data[i-4];
	}

	if(max_size < 4){
		return 0;
	}else{
		return data.data_length+4;
	}

}

size_t encode_COBS_bytes(const DataPacket &data,uint8_t *output,size_t max_size){
	uint8_t bytes_data[data.data_length+4] = {0};
	encode_bytes(data,bytes_data,sizeof(bytes_data));
	return encode_COBS(bytes_data,sizeof(bytes_data),output,max_size);
}

bool decode_bytes(const uint8_t *input,const size_t input_size,DataPacket &data){
	if(input_size >=4 && input_size <= 8){
		data.is_request = (input[0] >> 3) &0x1;
		data.priority = input[0] & 0x7;
		data.data_type = (DataType)((input[1]>>4)&0xF);
		data.board_ID = input[1] &0xF;
		data.register_ID = (input[2]<<8) | input[3];
		data.data_length = input_size-4;
		for(size_t i = 0; i<(input_size-4); i++){
			data.data[i] = input[i+4];
		}
		return true;
	}else{
		return false;
	}
}

bool decode_COBS_bytes(const uint8_t *input,DataPacket &data){
	uint8_t decoded_bytes[8] = {0};
	size_t decoded_size = decode_COBS(input,decoded_bytes,sizeof(decoded_bytes));
	if(4 <= decoded_size && decoded_size <=8){
		decode_bytes(decoded_bytes,decoded_size,data);
		return true;
	}else{
		return false;
	}
}

size_t encode_COBS(const uint8_t *input, size_t input_size, uint8_t *output, size_t output_size_limit) {
    uint8_t count = 0; //次にsource_data[i]に0x00が出るまでの配列番号をカウント
    int mark = 0; //最後に0x00が出たsource_data[i]の配列番号をキープ

    for (size_t i = 0 ; i < input_size + 1; i++) {

        //現在チェックしているsource_data[i]の中身が0x00ではない場合
        if (input[i] != 0x00) {
            output[i + 1] = input[i]; //チェックした値をそのままcobs配列に書き込む
            count ++;//前回0x00が出たsource_data[i]の配列内のカウント値を+1
        }else { //現在チェックしているsource_data[i]の中身が0x00だった場合
            count ++;//前回0x00が出た配列のカウントを+1
            output[mark] = count;//前回0x00が出たcobs配列にカウント値を書き込み確定
            mark = i + 1; // 現在のcobs配列を0x00が出た配列としてキープ
            count = 0 ;//0x00が出た配列のカウントをリセット
            output[mark] = count; //今回0x00が出たcobs配列にカウント値を書き込む
        }
    }

    //末端処理
    output[mark] = count; //前回00が出た配列にカウント値を改めて書き込む
    output[input_size + 1] = 0; //終端に00を代入する
    return input_size + 2;
}

size_t decode_COBS(const uint8_t *input, uint8_t *output, size_t output_size_limit) {
    size_t input_size;
    for(input_size = 0; input_size < output_size_limit; input_size++){
        if(input[input_size+1]==0){
            break;
        }
        output[input_size] = input[input_size + 1];
    }

    //該当する場所に0x00を代入する処理
    size_t i = 0;
    while ( i <= input_size) {
        i = i + input[i];
        output[i - 1] = 0;
    }

    return input_size;
}

}
}
