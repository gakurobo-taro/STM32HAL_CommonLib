/*
 * pwm.hpp
 *
 *  Created on: 2023/12/23
 *      Author: yaa3k
 */

#ifndef PWM_HPP_
#define PWM_HPP_

#include "main.h"

namespace G24_STM32HAL::CommonLib{

class IPWM{
public:
	 virtual void  out(float val) = 0;
};

////////////////////////////////////////////////////////////
//Hard ware PWM class///////////////////////////////////////
////////////////////////////////////////////////////////////
class PWMHard:public IPWM{
private:
	TIM_HandleTypeDef *tim;
	const uint32_t ch;
	const float min;
	const float max;
	const float diff_inv;

public:
	PWMHard(TIM_HandleTypeDef *_tim,uint32_t _ch,float _min = 0,float _max = 1)
		: tim(_tim),ch(_ch),min(_min),max(_max),diff_inv(1/(max - min)){
	}

	void out(float val) override;
	void out_as_gpio(bool gpio_state);
	void out_as_gpio_toggle(void);

	uint32_t get_compare_val(void){
		return __HAL_TIM_GET_COMPARE(tim, ch);
	}
	void start(void){
		HAL_TIM_PWM_Start(tim, ch);
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}
	void stop(void){
		HAL_TIM_PWM_Stop(tim, ch);
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}

};

////////////////////////////////////////////////////////////
//Soft ware PWM class(using LL library)/////////////////////
////////////////////////////////////////////////////////////
#ifdef USE_GPIO_LL

class PWMLLSoft:public IPWM{
private:
	GPIO_TypeDef *port;
	const uint16_t pin;
	const float min;
	const float max;
	const float diff_inv;

	uint16_t count = 0;
	uint16_t period = 0;
	uint16_t duty = 0xFFFF;
	bool output_state = false;

public:
	PWMLLSoft(GPIO_TypeDef *_port,uint16_t _pin,float _min = 0,float _max = 1)
		: port(_port),pin(_pin),min(_min),max(_max),diff_inv(1/(max - min)){
	}

	void set_input_mode(bool mode){
		if(mode) LL_GPIO_SetPinMode(port, pin, LL_GPIO_MODE_INPUT);
		else LL_GPIO_SetPinMode(port, pin, LL_GPIO_MODE_OUTPUT);
	}

	void out(float val)override{
		if(val < min || max < val)val  = 0;
		duty = (val - min)*diff_inv*period;
	}

	void set_duty(const uint16_t _duty){ duty = _duty; }
	uint16_t get_duty(void){ return duty; }

	void set_period(uint16_t _period){ count = 0; period = _period; }
	uint16_t get_period(void){ return period; }

	bool get_input_state(void){ return LL_GPIO_IsInputPinSet(port,pin); }
	void set_output_state(bool state){ output_state = state; }

	//timer interrupt function
	void update(void){
		count = count >= period ? 0 : count+1;
		if(count < duty && output_state) LL_GPIO_SetOutputPin(port,pin);
		else LL_GPIO_ResetOutputPin(port,pin);
	}
};
#endif

}


#endif /* PWM_HPP_ */
