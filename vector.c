#include <stdlib.h>
#include <string.h>

//Wrote the most generic function i could ever come up with for a vector_push
//copies the memory and increases the vector len
void vector_push(char* vector, const char* element, size_t* vector_len, size_t element_size){
	memcpy(vector+((*vector_len)*element_size), element, element_size);
	++(*vector_len);
}

//Copies the memory of vector into tmp, loops and looks for the desire element, removes it by copying
//tmp over vector with 1 element of advantage so basically the desired element gets overwritten with the next element
//Decrements the vector_len
void vector_remove(char* vector, const char* element, size_t* vector_len, size_t element_size){
	size_t i;
	char tmp[vector_len*element_size];
	
	memcpy(tmp, vector, vector_len*element_size);
	
	for(i=0; i<(*vector_len); ++i){
		if(memcmp(tmp+(i*element_size), element, element_size) == 0){
			memcpy(vector+(i*element_size), tmp+((i+1)*element_size), (*vector_len-1-i)*element_size);
			--(*vector_len);
		}
	}
}