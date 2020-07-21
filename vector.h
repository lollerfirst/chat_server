#ifndef __VECTOR_H_
#define __VECTOR_H_ 1

extern void vector_push(void* vector, const void*  element, size_t vector_len, size_t element_size);
extern void vector_remove(void* vector, const void*  element, size_t vector_len, size_t element_size);

#endif