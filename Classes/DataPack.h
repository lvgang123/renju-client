#ifndef __DATAPACK_H__
#define __DATAPACK_H__

#include "Message.h"

/*
 * ����Ϣ�ķ�����
 */
class DataPack
{
public:
	static Message * Pack(const char * data);
	static Message * Unpack(const char * buf);
};

#endif