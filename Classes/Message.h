#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <memory.h>

// ���ݰ���ʽ
class Message 
{
public:
	// ��Ϣ�峤��
	unsigned int msgLen;

	// ���ʵķ���ID
	unsigned int serviceID;

	// ���ʵķ�������Ľӿڵ�ID
	unsigned int methodID;
	
	// ��Ϣ��
	char data[];

	// ��ȡ��Ϣ�����ֽ���
	const int GetMsgLenBinaryCnt() const { return 4; }

	// ��ȡserviceID�ֽ���
	const int GetServiceIDBinaryCnt() const { return 4; }

	// ��ȡmethodID�ֽ���
	const int GetMethodIDBinaryCnt() const { return 4; }

	// ��ȡ��Ϣͷ�����ֽ���
	const int GetMsgHeadBinaryCnt() const { return 12; }

	// GET
	const int GetMsgLen() { return msgLen; }
	const int GetServiceID() { return serviceID; }
	const int GetMethodID() { return methodID; }

	// SET
	void SetMsgLen(unsigned int msgLen) { this->msgLen = msgLen; }
	void SetServiceID(unsigned int serviceID) { this->serviceID = serviceID; }
	void SetMethodID(unsigned int methodID) { this->methodID = methodID; }
	void SetData(char * data) { memcpy(this->data, data, this->msgLen); }
};

#endif //�� __MESSAGE_H__