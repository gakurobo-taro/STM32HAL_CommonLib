/*
 * byte_reader_writer.hpp
 *
 *  Created on: Dec 28, 2023
 *      Author: yaa3k
 */

#ifndef BYTE_READER_WRITER_HPP_
#define BYTE_READER_WRITER_HPP_

#include <optional>
#include <string.h>

namespace G24_STM32HAL::CommonLib{

class ByteWriter{
  uint8_t *const begin;
  uint8_t *const end;
  size_t &written_size;
public:
  ByteWriter(uint8_t* _begin, uint8_t* _end,size_t &_written_size): begin(_begin),end(_end),written_size(_written_size){
	  written_size = 0;
  }
  ByteWriter(uint8_t* _bytes, size_t sz,size_t &_written_size): begin(_bytes),end(_bytes+sz),written_size(_written_size){
	  written_size = 0;
  }

  template <class T>
  bool write(const T& value){
    if (written_size+begin+sizeof(T) > end){
      return false;
    }
    memcpy(begin+written_size, &value,sizeof(T));
    written_size += sizeof(T);
    return true;
  }
};

class ByteReader{
  const uint8_t *const begin;
  const uint8_t *const end;
  const uint8_t* iter;
public:
  ByteReader(const uint8_t* _begin, const uint8_t* _end): begin(_begin),end(_end),iter(_begin){}
  ByteReader(const uint8_t* _bytes, size_t sz): begin(_bytes),end(_bytes+sz),iter(_bytes){}

  template <class T>
  std::optional<T> read(){
    T value;
    if (iter+sizeof(T) > end){
      return std::nullopt;
    }
    memcpy(&value,iter, sizeof(T));
    iter += sizeof(T);
    return value;
  }
};

}

#endif /* BYTE_READER_WRITER_HPP_ */
