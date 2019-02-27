#include "rb.h"

void RingBuffer_ClearGetTail(RingBuffer_data_t* buf)
{
    do
    {
        buf->buffer_tail = buf->buffer_head;

    } while (buf->buffer_tail != buf->buffer_head);
}

void RingBuffer_ClearPutHead(RingBuffer_data_t* buf)
{
    do
    {
        buf->buffer_head = buf->buffer_tail;

    } while (buf->buffer_tail != buf->buffer_head);
}

int32_t RingBuffer_GetRoom(RingBuffer_data_t* buf)
{
    int32_t retval = ((buf->buffer_tail + buf->conf.size) - buf->buffer_head) % buf->conf.size;
    return retval == 0 ? buf->conf.size : retval;
}

int32_t RingBuffer_GetData(RingBuffer_data_t* buf)
{
    int32_t retval = ((buf->buffer_head + buf->conf.size) - buf->buffer_tail) % buf->conf.size;
    return retval;
}

bool RingBuffer_PutSamples(RingBuffer_data_t* buf, void* samples, int32_t len)
{
    bool retval = false;
    if (len < RingBuffer_GetRoom(buf))
    {
        int32_t maxupper = buf->conf.size - buf->buffer_head;
        int32_t copylower = len - maxupper;
        int32_t copyupper = copylower > 0 ? maxupper : len;
        memcpy((char*)buf->conf.buffer + (buf->buffer_head * buf->conf.sizeofItem),(char*)samples,copyupper * buf->conf.sizeofItem);
        if (copylower > 0)
        {
            memcpy((char*)buf->conf.buffer,(char*)samples + (maxupper * buf->conf.sizeofItem),copylower * buf->conf.sizeofItem);
        }


        buf->buffer_head =  (buf->buffer_head + len) % buf->conf.size;
        retval = true;
    }
    return retval;
}

bool RingBuffer_GetSamples(RingBuffer_data_t* buf, void* samples, int32_t len)
{
    bool retval = false;
    if (len <= RingBuffer_GetData(buf))
    {
        int32_t maxupper = buf->conf.size - buf->buffer_tail;
        int32_t copylower = len - maxupper;
        int32_t copyupper = copylower > 0 ? maxupper : len;
        memcpy((char*)samples,(char*)buf->conf.buffer + (buf->buffer_tail * buf->conf.sizeofItem),copyupper * buf->conf.sizeofItem);
        if (copylower > 0)
        {
            memcpy(((char*)samples) + (maxupper * buf->conf.sizeofItem), (char*)buf->conf.buffer,copylower * buf->conf.sizeofItem);
        }

        buf->buffer_tail =  (buf->buffer_tail + len) % buf->conf.size;
        retval = true;
    }
    return retval;
}
