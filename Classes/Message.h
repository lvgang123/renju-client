#ifndef __MESSAGE_H__
#define __MESSAGE_H__

// ���ݰ���ʽ
class Message 
{
public:
	// ��Ϣ�峤��
	unsigned int msgLen;

	// ��ϢID
	unsigned int msgID;
	
	// ��Ϣ��
	char data[];

	// ��ȡ��Ϣ�����ֽ���
	const int GetMsgLenBinaryCnt() const { return 4; }

	// ��ȡ��ϢID�ֽ���
	const int GetMsgIDBinaryCnt() const { return 4; }
};

#endif //�� __MESSAGE_H__