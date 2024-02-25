/*
 * id_map_control.hpp
 *
 *  Created on: Feb 25, 2024
 *      Author: yaa3k
 */

#ifndef ID_MAP_CONTROL_HPP_
#define ID_MAP_CONTROL_HPP_

#include "byte_reader_writer.hpp"
#include <functional>

namespace G24_STM32HAL::CommonLib{

	class DataManager{
	private:
		std::function<bool(CommonLib::ByteReader&)> f_set;
		std::function<bool(CommonLib::ByteWriter&)> f_get;
	public:
		DataManager(std::function<bool(CommonLib::ByteReader&)>&& _write,
				std::function<bool(CommonLib::ByteWriter&)>&&_read):
			f_set(_write),f_get(_read){}

		//byte->ref
		bool set(CommonLib::ByteReader& r){ return f_set ? f_set(r) : false; }
		//ref->byte
		bool get(CommonLib::ByteWriter& w){ return f_get ? f_get(w) : false; }

		template<class T> static DataManager generate(T& ref){
			auto readf = [&](CommonLib::ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					ref = val.value();
					return true;
				}else{
					return false;
				}
			};
			auto writef = [&](CommonLib::ByteWriter& w){
				w.write<T>(ref);
				return true;
			};
			return DataManager(readf,writef);
		}

		template<class T> static DataManager generate(std::function<void(T)>&& setter,std::function<T(void)>&& getter){
			auto readf = [&](CommonLib::ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					setter(val.value());
					return true;
				}else{
				   return false;
				}
			};
			auto writef = [&](CommonLib::ByteWriter& w){
				w.write(getter());
				return true;
			};
			return DataManager(readf,writef);
		}
		template<class T> static DataManager generate(std::function<void(T)>&& setter){
			auto readf = [&](CommonLib::ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					setter(val.value());
					return true;
				}else{
				   return false;
				}
			};
			return DataManager(readf,nullptr);
		}
		template<class T> static DataManager generate(std::function<T(void)>&& getter){
			auto writef = [&](CommonLib::ByteWriter& w){
				w.write(getter());
				return true;
			};
			return DataManager(nullptr,writef);
		}
	};


	class IDMap{
	public:
		std::unordered_map<size_t, DataManager> managers_map;
		IDMap(std::unordered_map<size_t, DataManager>&& _managers_map):managers_map(_managers_map){}

		bool set(int id,CommonLib::ByteReader& r){
			auto iter=managers_map.find(id);
			if (iter!=managers_map.end()){
				return iter->second.set(r);
			}
			return false;
		}
		bool get(int id,CommonLib::ByteWriter& w){
			auto iter=managers_map.find(id);
			if (iter!=managers_map.end()){
				return iter->second.get(w);
			}
			return false;
		}
	};

	class IDMapBuilder{
	public:
		std::unordered_map<size_t, DataManager> managers_map;
		IDMapBuilder& add(size_t id,const DataManager& c){
			managers_map.insert(std::pair(id, c));
			return *this;
		}
		IDMap build(){
		   return IDMap(std::move(managers_map));
		}
	};


}


#endif /* ID_MAP_CONTROL_HPP_ */
