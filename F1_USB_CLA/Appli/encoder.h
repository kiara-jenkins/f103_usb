// interface quadratic encoder utilisant un timer

#ifdef __cplusplus
extern "C" {
#endif

void encoder_init( TIM_TypeDef * Timer );

__STATIC_INLINE int16_t encoder_get( TIM_TypeDef *TIMx )
{
return (int16_t)TIMx->CNT;
}

#ifdef __cplusplus
} // extern "C"
#endif
