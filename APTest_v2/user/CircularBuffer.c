/*
 * CircularBuffer.c
 *
 *  Created on: Sep 17, 2015
 *      Author: uds002
 */

#include <CircularBuffer.h>
#include <mem.h>


uint8_t create(circular_buffer_t * c_buffer, uint32_t length) {
	c_buffer->buffer = (uint8_t *)os_malloc(length);
	if(c_buffer->buffer != NULL)
		return 1;
	return 0;
}

uint32_t read(circular_buffer_t * c_buffer, uint8_t *data, uint32_t length)
{
    uint32_t n = 0;
    while(n < length && getSize(c_buffer) > 0)
    {
        data[n++] = c_buffer->buffer[c_buffer->readIndex++];
        if(c_buffer->readIndex == BUFFER_LENGTH)
        	c_buffer->readIndex = 0;
        --c_buffer->bytesAvailable;
    }

    return n;
}



uint32_t write(circular_buffer_t * c_buffer, uint8_t *data, uint32_t length)
{
    uint32_t n = 0;
    while(n < length && getSize(c_buffer) < BUFFER_LENGTH)
    {
    	c_buffer->buffer[c_buffer->writeIndex++] = data[n++];
        if(c_buffer->writeIndex == BUFFER_LENGTH)
        	c_buffer->writeIndex = 0;
        ++c_buffer->bytesAvailable;
    }

    return n;
}

uint32_t copy(circular_buffer_t * c_buffer, uint8_t *data) {
	uint32_t n = 0, size;
	uint16_t idx;
	idx = c_buffer->readIndex;
	size = getSize(c_buffer);
	while( n < size ) {
		data[n++] = c_buffer->buffer[idx++];
		if(idx == BUFFER_LENGTH)
			idx = 0;
	}
	return size;
}


uint32_t getCapacity(circular_buffer_t * c_buffer) {
    return BUFFER_LENGTH;
}


uint32_t getSize(circular_buffer_t * c_buffer) {
    return c_buffer->bytesAvailable;
}


uint8_t isEmpty(circular_buffer_t * c_buffer) {
    return getSize(c_buffer) == 0;
}

uint8_t isFull(circular_buffer_t * c_buffer) {
    return getSize(c_buffer) == BUFFER_LENGTH;
}
