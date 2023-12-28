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

enum class BuffSize:size_t{
	SIZE2 = 1,
	SIZE4,
	SIZE8,
	SIZE16,
	SIZE32,
	SIZE64,
	SIZE128,
};

template<typename T, size_t n>
class RingBuffer{
private:
	const size_t SIZE = 1<<n;
	const size_t MASK = SIZE-1;
    int head = 0;
    int tail = 0;
    int data_count = 0;

    T data_buff[1<<n] = {0};
public:
    void push(const T &input){
        data_buff[head] = input;
        head = (head+1) & MASK;
        data_count ++;
        if(data_count > SIZE){
            data_count = SIZE;
            tail = head;
        };
    }

    bool pop(T &output){
        if(data_count > 0){
            output = data_buff[tail];
            tail = (tail + 1) & MASK;
            data_count --;
            if(data_count < 0) data_count = 0;
            return true;
        }else{
            return false;
        }
    }

    int get_free_level(void){
        return SIZE - data_count;
    }
    int get_busy_level(void){
        return data_count;
    }
    void reset(void){
        head = 0;
        tail = 0;
        data_count = 0;
    }

};

}


#endif /* RING_BUFFER_HPP_ */
