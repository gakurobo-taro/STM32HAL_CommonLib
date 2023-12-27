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
	 virtual void  start(void) = 0;
	 virtual void  stop(void) = 0;
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
	float diff_inv;

public:
	PWMHard(TIM_HandleTypeDef *_tim,uint32_t _ch,float _min = 0,float _max = 1)
		: tim(_tim),ch(_ch),min(_min),max(_max){
		diff_inv = 1/(max - min);
	}

	void out(float val) override;
	void out_as_gpio(bool gpio_state);
	void out_as_gpio_toggle(void);

	//inline functions
	uint32_t get_compare_val(void){
		return __HAL_TIM_GET_COMPARE(tim, ch);
	}
	void start(void)override{
		HAL_TIM_PWM_Start(tim, ch);
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}
	void stop(void)override{
		HAL_TIM_PWM_Stop(tim, ch);
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}

};

}


#endif /* PWM_HPP_ */
