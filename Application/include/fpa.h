/** @file */

#ifndef FPA_H
#define FPA_H


/**@{
 *
 * Fixed Point Arithmetic macro.
 *
 * @param[in] nNumber to multiply.
 *
 * @param[in] t   Times to multiply.
 *
 * @param[in] f   Fractional bits.
 */
#define FPA_MUL_UI8(n, t, f)  ( (uint8_t)(((uint16_t)( (uint8_t)n * (t))) >> (f)))
#define FPA_MUL_UI16(n, t, f) ((uint16_t)(((uint32_t)((uint16_t)n * (t))) >> (f)))
#define FPA_MUL_UI32(n, t, f) ((uint32_t)(((uint64_t)((uint32_t)n * (t))) >> (f)))

#define FPA_MUL_I8(n, t, f)   (  (int8_t)( ((int16_t)(  (int8_t)n * (t))) >> (f)))
#define FPA_MUL_I16(n, t, f)  ( (int16_t)( ((int32_t)( (int16_t)n * (t))) >> (f)))
#define FPA_MUL_I32(n, t, f)  ( (int32_t)( ((int64_t)( (int32_t)n * (t))) >> (f)))
/**@}*/ 

#endif /* FPA_H */
