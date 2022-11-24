#ifndef FPA_H
#define FPA_H

#define FPA_UI8(x, m, s)  ( (uint8_t)(((uint16_t)( (uint8_t)x * m)) >> s))
#define FPA_UI16(x, m, s) ((uint16_t)(((uint32_t)((uint16_t)x * m)) >> s))
#define FPA_UI32(x, m, s) ((uint32_t)(((uint64_t)((uint32_t)x * m)) >> s))

#define FPA_I8(x, m, s)   (  (int8_t)( ((int16_t)(  (int8_t)x * m)) >> s))
#define FPA_I16(x, m, s)  ( (int16_t)( ((int32_t)( (int16_t)x * m)) >> s))
#define FPA_I32(x, m, s)  ( (int32_t)( ((int64_t)( (int32_t)x * m)) >> s))

#endif /* FPA_H */
