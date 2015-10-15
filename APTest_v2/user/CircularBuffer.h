/*
 * CircularBuffer.h
 *
 */

#ifndef USER_CIRCULARBUFFER_H_
#define USER_CIRCULARBUFFER_H_

#include <os_type.h>

#define BUFFER_LENGTH	3072

typedef struct {
	uint16_t readIndex;
	uint16_t writeIndex;
	uint8_t * buffer;
	uint32_t length;
	size_t bytesAvailable;
}circular_buffer_t;

uint8_t create(circular_buffer_t * c_buffer, uint32_t length);
uint32_t read(circular_buffer_t * c_buffer, uint8_t *data, uint32_t length);
uint32_t write(circular_buffer_t * c_buffer, uint8_t *data, uint32_t length);
uint32_t copy(circular_buffer_t * c_buffer, uint8_t *data);
uint32_t getCapacity(circular_buffer_t * c_buffer);
uint32_t getSize(circular_buffer_t * c_buffer);
uint8_t isEmpty(circular_buffer_t * c_buffer);
uint8_t isFull(circular_buffer_t * c_buffer);


#endif /* USER_CIRCULARBUFFER_H_ */
