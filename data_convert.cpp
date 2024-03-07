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

	can_frame.id = ((data.priority&0xF)<<PRIORITY_BIT) | (((uint8_t)data.data_type&0xF)<<DATA_TYPE_BIT)
			| ((data.board_ID&0xF)<<BOARD_ID_BIT) | (data.register_ID&0xFFFF);

	can_frame.data_length = data.data_length;
	memcpy(can_frame.data, data.data,data.data_length);
	return true;

}
bool decode_can_frame(const CanFrame &can_frame,DataPacket &data){
	if(can_frame.is_ext_id){
		data.is_request = can_frame.is_remote;
		data.priority = (can_frame.id>>PRIORITY_BIT)&0xF;
		data.data_type = (DataType)((can_frame.id >> DATA_TYPE_BIT)&0xF);
		data.board_ID = (can_frame.id >> BOARD_ID_BIT)&0xF;
		data.register_ID = can_frame.id & 0xFFFF;

		memcpy(data.data, can_frame.data,can_frame.data_length);
		data.data_length = can_frame.data_length;

		return true;
	}else{
		return false;
	}

}

size_t encode_bytes(const DataPacket &data,uint8_t *output,size_t max_size){
	if(max_size >= 4){
		output[0] = data.priority&0xF;
		output[1] = (((uint8_t)data.data_type&0xF) << 4)|(data.board_ID&0xF);
		output[2] = (data.register_ID>>8)&0xFF;
		output[3] = data.register_ID & 0xFF;
	}
	memcpy(&output[4], data.data,data.data_length);

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
	if(input_size >=4 && input_size <= 12){
		data.is_request = (input[0] >> 4) &0x1;
		data.priority = input[0] & 0xF;
		data.data_type = (DataType)((input[1]>>4)&0xF);
		data.board_ID = input[1] &0xF;
		data.register_ID = (input[2]<<8) | input[3];
		data.data_length = input_size-4;
		memcpy(data.data, &input[4],input_size-4);
		return true;
	}else{
		return false;
	}
}

bool decode_COBS_bytes(const uint8_t *input,DataPacket &data){
	uint8_t decoded_bytes[12] = {0};
	size_t decoded_size = decode_COBS(input,decoded_bytes,sizeof(decoded_bytes));
	if(4 <= decoded_size && decoded_size <=8){
		decode_bytes(decoded_bytes,decoded_size,data);
		return true;
	}else{
		return false;
	}
}

size_t can_to_slcan(const CanFrame &frame,char *str,const size_t str_max_size){
	if(str_max_size < SLCAN_STR_MAX_SIZE){
		return 0;
	}
	size_t head = 0;
	//frame type(0)
	if(frame.is_remote){
		if(frame.is_ext_id){
			str[head] = 'R';
		}else{
			str[head] = 'r';
		}
	}else{
		if(frame.is_ext_id){
			str[head] = 'T';
		}else{
			str[head] = 't';
		}
	}

	//ID
	if(frame.is_ext_id){
		for(head = 1; head < 9; head++){
			str[head] = (frame.id >> 4*(8-head)) & 0xF;
			if(str[head] < 10){
				str[head] += '0';
			}else{
				str[head] += 'A'-10;
			}
		}
	}else{
		for(head = 1; head < 4; head++){
			str[head] = (frame.id >> 4*(3-head)) & 0xF;
			if(str[head] < 10){
				str[head] += '0';
			}else{
				str[head] += 'A'-10;
			}
		}
	}

	//DLC
	str[head++] = frame.data_length + '0';

	if(frame.is_remote){
		str[++head] = '\r';
	}else{
		for(size_t i = 0; i < frame.data_length; i++){
			str[i*2 + head] = (frame.data[i] >> 4)&0xF;

			if(str[i*2 + head] < 10){
				str[i*2 + head] += '0';
			}else{
				str[i*2 + head] += 'A'-10;
			}

			str[i*2 + head+1] = frame.data[i] & 0xF;
			if(str[i*2 + head+1] < 10){
				str[i*2 + head+1] += '0';
			}else{
				str[i*2 + head+1] += 'A'-10;
			}
		}
		head += frame.data_length*2;

		str[head] = '\r';
	}

	return ++head;
}
bool slcan_to_can(const char *str, CanFrame &frame){
	int head = 0;
	switch(str[head]){
	case 't':
		frame.is_ext_id = false;
		frame.is_remote = false;
		break;
	case 'T':
		frame.is_ext_id = true;
		frame.is_remote = false;
		break;
	case 'R':
		frame.is_remote = true;
		frame.is_ext_id = true;
		break;
	case 'r':
		frame.is_remote = true;
		frame.is_ext_id= false;
		break;
	default:
		return false;
		break;
	}
	//ID
	if(frame.is_ext_id){
		for(head = 1; head < 9; head++){
			int tmp = 0;
			if(str[head] >= 'A'){
				tmp = (str[head] - 'A'+10) & 0xF;
			}else{
				tmp = (str[head] - '0') & 0xF;
			}

			frame.id |= tmp << 4*(8-head);
		}
	}else{
		for(head = 1; head < 4; head++){
			int tmp = 0;
			if(str[head] >= 'A'){
				tmp = (str[head] - 'A'+10) & 0xF;
			}else{
				tmp = (str[head] - '0') & 0xF;
			}

			frame.id |= tmp << 4*(3-head);
		}
	}

	//DLC
	frame.data_length = str[head++]&0xF;

	//data
	if(frame.is_remote){

	}else{
		for(size_t i = 0; i < frame.data_length; i ++){
			int tmp1 = str[head + 2*i];
			int tmp2 = str[head + 1 + 2*i];
			if(tmp1 >= 'A'){
				tmp1 = (tmp1 - 'A'+10) & 0xF;
			}else{
				tmp1 = (tmp1 - '0') & 0xF;
			}

			if(tmp2 >= 'A'){
				tmp2 = (tmp2 - 'A'+10) & 0xF;
			}else{
				tmp2 = (tmp2 - '0') & 0xF;
			}

			frame.data[i] = (tmp1 << 4) | tmp2;
		}
	}
	return true;
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
