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

template<typename T>
class IRingBuffer{
public:
	bool virtual push(const T& input) = 0;
	bool virtual pop(T& output) = 0;
	size_t virtual get_free_level(void)const = 0;
	size_t virtual get_busy_level(void)const = 0;
	void virtual reset(void) = 0;
};

template<typename T, size_t n>
class RingBuffer : public IRingBuffer<T>{
private:
	const size_t SIZE = 1<<n;
	const size_t MASK = SIZE-1;
    size_t head = 0;
    size_t tail = 0;
    size_t data_count = 0;

    T data_buff[1<<n] = {0};
public:
    bool push(const T &input)override{
        data_buff[head] = input;
        head = (head+1) & MASK;
        data_count ++;
        if(data_count > SIZE){
            data_count = SIZE;
            tail = head;
            return false;
        };
        return true;
    }

    bool pop(T &output)override{
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

    size_t get_free_level(void)const override{
        return SIZE - data_count;
    }
    size_t get_busy_level(void)const override{
        return data_count;
    }
    void reset(void)override{
        head = 0;
        tail = 0;
        data_count = 0;
    }

};

}


#endif /* RING_BUFFER_HPP_ */
