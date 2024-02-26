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

	class DataAccessor{
	private:
		std::function<bool(ByteReader&)> f_set;
		std::function<bool(ByteWriter&)> f_get;
	public:
		DataAccessor(std::function<bool(ByteReader&)>&& _write,
				std::function<bool(ByteWriter&)>&&_read):
			f_set(_write),f_get(_read){}

		//byte->ref
		bool set(ByteReader& r){ return f_set ? f_set(r) : false; }
		//ref->byte
		bool get(ByteWriter& w){ return f_get ? f_get(w) : false; }

		template<class T> static DataAccessor generate(T& ref){
			auto readf = [&](ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					ref = val.value();
					return true;
				}else{
					return false;
				}
			};
			auto writef = [&](ByteWriter& w){
				w.write<T>(ref);
				return true;
			};
			return DataAccessor(readf,writef);
		}

		template<class T> static DataAccessor generate(std::function<void(T)>&& setter,std::function<T(void)>&& getter){
			auto readf = [&](ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					setter(val.value());
					return true;
				}else{
				   return false;
				}
			};
			auto writef = [&](ByteWriter& w){
				w.write(getter());
				return true;
			};
			return DataAccessor(readf,writef);
		}
		template<class T> static DataAccessor generate(std::function<void(T)>&& setter){
			auto readf = [&](ByteReader& r){
				std::optional<T> val = r.read<T>();
				if(val.has_value()){
					setter(val.value());
					return true;
				}else{
				   return false;
				}
			};
			return DataAccessor(readf,nullptr);
		}
		template<class T> static DataAccessor generate(std::function<T(void)>&& getter){
			auto writef = [&](ByteWriter& w){
				w.write(getter());
				return true;
			};
			return DataAccessor(nullptr,writef);
		}
	};


	class IDMap{
	public:
		std::unordered_map<size_t, DataAccessor> Accessors_map;
		IDMap(std::unordered_map<size_t, DataAccessor>&& _Accessors_map):Accessors_map(_Accessors_map){}

		bool set(int id,ByteReader& r){
			auto iter=Accessors_map.find(id);
			if (iter!=Accessors_map.end()){
				return iter->second.set(r);
			}
			return false;
		}
		bool get(int id,ByteWriter& w){
			auto iter=Accessors_map.find(id);
			if (iter!=Accessors_map.end()){
				return iter->second.get(w);
			}
			return false;
		}
	};

	class IDMapBuilder{
	public:
		std::unordered_map<size_t, DataAccessor> Accessors_map;
		IDMapBuilder& add(size_t id,const DataAccessor& c){
			Accessors_map.insert(std::pair(id, c));
			return *this;
		}
		IDMap build(){
		   return IDMap(std::move(Accessors_map));
		}
	};


}


#endif /* ID_MAP_CONTROL_HPP_ */
