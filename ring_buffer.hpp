/*
 * ring_buffer.hpp
 *
 *  Created on: 2023/12/21
 *      Author: yaa3k
 */

#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_

#include <stdbool.h>

namespace G24_STM32HAL::CommonLib{

enum class BuffSize{
	SIZE2 = 2,
    SIZE4 = 4,
    SIZE8 = 8,
    SIZE16 = 16,
    SIZE32 = 32,
    SIZE64 = 64,
	SIZE128 = 128,
};

template<typename T, BuffSize SIZE>
class RingBuffer{
private:
    int head = 0;
    int tail = 0;
    T data_buff[(int)SIZE];
    bool data_is_free[(int)SIZE];
public:
    RingBuffer(){
        for(int i = 0; i < (int)SIZE; i++){
            data_is_free[i] = true;
        }
    }
    bool push(const T &input){
        data_buff[head] = input;
        bool tmp = data_is_free[head];
        data_is_free[head] = false;

        head = (head+1) & ((int)SIZE-1);

        //既に存在していたデータを上書きしていたらfalse
        return tmp;
    }

    bool pop(T &output){
        if(data_is_free[tail]){
            tail = (tail + 1) & ((int)SIZE-1);
            return false;
        }else{
            output = data_buff[tail];
            data_is_free[tail] = true;
            tail = (tail + 1) & ((int)SIZE-1);
            return true;
        }
    }

    int get_free_level(void){
        int count = 0;
        for(int i = 0; i < (int)SIZE; i++){
            if(data_is_free[i]) count ++;
        }
        return count;
    }
    int get_busy_level(void){
    	int count = 0;
    	for(int i = 0; i < (int)SIZE; i++){
    		if(!data_is_free[i]) count ++;
    	}
    	return count;
    }
    void free(void){
    	for(int i = 0; i < (int)SIZE;i ++){
    		data_is_free[i] = true;
    	}
    }

};

}


#endif /* RING_BUFFER_HPP_ */
