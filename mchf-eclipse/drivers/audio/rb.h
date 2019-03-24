#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__


#include "uhsdr_types.h"

typedef struct {
    int32_t  size;
    int32_t  sizeofItem;
    void*  buffer; //buffer for filtered PCM data from Recv.
} RingBuffer_conf_t;

typedef struct {
    volatile uint16_t buffer_tail;
    volatile uint16_t buffer_head;
    const RingBuffer_conf_t conf;
} RingBuffer_data_t;

#define RingBuffer_Declare(name, type) typedef type name##_item_t; extern RingBuffer_data_t name;

#define RingBuffer_Define(name, buf_size) name##_item_t name##_buffer[(buf_size)]; RingBuffer_data_t name = { .buffer_tail = 0, .buffer_head = 0, .conf = { .size = (buf_size), .sizeofItem = sizeof(name##_item_t) , .buffer = name##_buffer  } };
#define RingBuffer_DefineExtMem(name, buf_size, buf_addr) RingBuffer_data_t name = { .buffer_tail = 0, .buffer_head = 0, .conf = { .size = (buf_size), .sizeofItem = sizeof(name##_item_t) , .buffer = (buf_addr)  } };

void RingBuffer_ClearGetTail(RingBuffer_data_t* buf);
void RingBuffer_ClearPutHead(RingBuffer_data_t* buf);
int32_t RingBuffer_GetRoom(RingBuffer_data_t* buf);
int32_t RingBuffer_GetData(RingBuffer_data_t* buf);
bool RingBuffer_PutSamples(RingBuffer_data_t* buf, void* samples, int32_t len);
bool RingBuffer_GetSamples(RingBuffer_data_t* buf, void* samples, int32_t len);



#endif
