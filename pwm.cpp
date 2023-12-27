/*
 * pwm.cpp
 *
 *  Created on: 2023/12/23
 *      Author: yaa3k
 */

#include "pwm.hpp"

namespace G24_STM32HAL::CommonLib{


void PWMHard::out(float val){

	if(val < min || max < val)val  = 0;

	__HAL_TIM_SET_COMPARE(tim, ch, (val - min)*diff_inv*tim->Init.Period);
}

void PWMHard::out_as_gpio(bool gpio_state){
	if(gpio_state){
		__HAL_TIM_SET_COMPARE(tim, ch,tim->Init.Period);
	}else{
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}
}
void PWMHard::out_as_gpio_toggle(void){
	if(get_compare_val()){
		__HAL_TIM_SET_COMPARE(tim, ch,0);
	}else{
		__HAL_TIM_SET_COMPARE(tim, ch,tim->Init.Period);
	}
}

}
