/*
 * LED_control.hpp
 *
 *  Created on: Mar 9, 2024
 *      Author: gomas
 */

#ifndef LED_CONTROL_HPP_
#define LED_CONTROL_HPP_


#include "pwm.hpp"

#include "main.h"

namespace G24_STM32HAL::CommonLib{
	struct LEDState{
		bool state;
		uint16_t length;
	};

	class ILED{
	public:
		virtual void play(const LEDState *pattern) = 0;
		virtual bool is_playing(void) = 0;
	};

	class LEDPwm:public ILED{
	private:
		CommonLib::PWMHard pwm;

		float duty = 1;

		const LEDState *playing_pattern = nullptr;
		uint32_t pattern_count = 0;
		uint32_t length_count = 0;

	public:
		LEDPwm(TIM_HandleTypeDef *tim,uint32_t ch):pwm(tim,ch){
		}

		void start(void){pwm.start();}

		void play(const LEDState *pattern) override{
			playing_pattern = pattern;
			pattern_count = 0;
			length_count = 0;

			length_count = playing_pattern[pattern_count].length;

			pwm.out(playing_pattern[pattern_count].state?duty:0.0f);
		}

		bool is_playing(void)override{return playing_pattern!=nullptr ? true:false;}

		void update(void){
			if(playing_pattern != nullptr){
				length_count  --;
				if(length_count <= 0){
					pattern_count ++;

					if(playing_pattern[pattern_count].length == 0){
						playing_pattern = nullptr;
						pwm.out(0.0f);
						return;
					}
					length_count = playing_pattern[pattern_count].length;
					pwm.out(playing_pattern[pattern_count].state?duty:0.0f);
				}
			}else{

			}
		}

		void set_duty(float _duty){duty = _duty;};

	};
#ifdef USE_GPIO_LL
	class LEDLLGpio:public ILED{
	private:
		GPIO_TypeDef *port;
		const uint32_t pin;

		const LEDState *playing_pattern = nullptr;
		uint32_t pattern_count = 0;
		uint32_t length_count = 0;

	public:
		LEDLLGpio(GPIO_TypeDef *_port,uint32_t _pin):port(_port),pin(_pin){
		}

		void play(const LEDState *pattern) override{
			playing_pattern = pattern;
			pattern_count = 0;
			length_count = 0;

			length_count = playing_pattern[pattern_count].length;
			if(playing_pattern[pattern_count].state){
				LL_GPIO_SetOutputPin(port,pin);
			}else{
				LL_GPIO_ResetOutputPin(port,pin);
			}
		}

		bool is_playing(void)override{return playing_pattern!=nullptr ? true:false;}

		void update(void){
			if(playing_pattern != nullptr){
				length_count  --;
				if(length_count <= 0){
					pattern_count ++;

					if(playing_pattern[pattern_count].length == 0){
						playing_pattern = nullptr;
						HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);
						return;
					}
					length_count = playing_pattern[pattern_count].length;
					if(playing_pattern[pattern_count].state){
						LL_GPIO_SetOutputPin(port,pin);
					}else{
						LL_GPIO_ResetOutputPin(port,pin);
					}
				}
			}else{

			}
		}
	};
#else
	class LEDHALGpio:public ILED{
	private:
		GPIO_TypeDef *port;
		const uint32_t pin;

		const LEDState *playing_pattern = nullptr;
		uint32_t pattern_count = 0;
		uint32_t length_count = 0;

	public:
		LEDHALGpio(GPIO_TypeDef *_port,uint32_t _pin):port(_port),pin(_pin){
		}

		void play(const LEDState *pattern) override{
			playing_pattern = pattern;
			pattern_count = 0;
			length_count = 0;

			length_count = playing_pattern[pattern_count].length;

			HAL_GPIO_WritePin(port,pin,playing_pattern[pattern_count].state?GPIO_PIN_SET:GPIO_PIN_RESET);
		}

		bool is_playing(void)override{return playing_pattern!=nullptr ? true:false;}

		void update(void){
			if(playing_pattern != nullptr){
				length_count  --;
				if(length_count <= 0){
					pattern_count ++;

					if(playing_pattern[pattern_count].length == 0){
						playing_pattern = nullptr;
						HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);
						return;
					}
					length_count = playing_pattern[pattern_count].length;
					HAL_GPIO_WritePin(port,pin,playing_pattern[pattern_count].state?GPIO_PIN_SET:GPIO_PIN_RESET);
				}
			}else{

			}
		}
	};
#endif
}

#endif /* LED_CONTROL_HPP_ */
