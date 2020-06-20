#include "Net.h"
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <process.h>

#include "cocos2d.h"
USING_NS_CC;

SOCKET Net::m_connetSocket = INVALID_SOCKET;
bool Net::m_closed = true; // ��ʼ���׽���δ���ӣ����������ǹرյġ�
HANDLE Net::m_hRecvHandle = INVALID_HANDLE_VALUE;
HANDLE Net::m_hSendHandle = INVALID_HANDLE_VALUE;
Pool * Net::m_msgPool = new Pool();
queue<Message*> Net::m_recvQueue;
queue<Message*> Net::m_sendQueue;

/*
 * ���ӷ����
 *
 * ����ֵ��
 *  ���ӳɹ�����true�����򷵻�false��
 */
bool Net::Connect(const char* ip, short port)
{
	if (!m_closed) {
		// �׽����ѳ�ʼ���������еڶ�������
		return true;
	}

	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		CCLOG("WSAStartup failed: %d\n", iResult);
		return false;
	}

	m_connetSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_connetSocket == INVALID_SOCKET) {
		CCLOG("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	iResult = connect(m_connetSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (iResult == SOCKET_ERROR) {
		closesocket(m_connetSocket);
		CCLOG("Unable to connect to server: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	m_closed = false;
	return true;
}

/*
 * ����������������������߳�
 */
bool Net::Start()
{
	if (!StartSend()) {
		CCLOG("start send failed.");
		return false;
	}
	if (!StartRecv()) {
		CCLOG("start recv failed.");
		return false;
	}
	return true;
}

/*
 * ��������
 */
void Net::Send(unsigned int msgID, unsigned int msgLen, char * data)
{
	Message * msg = m_msgPool->Get(msgLen);
	msg->msgID = msgID;
	msg->msgLen = msgLen;
	memcpy(msg->data, data, msg->msgLen);

	// ���뷢�Ͷ�����
	m_sendQueue.push(msg);
}

/*
 * ��������
 *
 * ����ֵ
 *  �������ݶ����������ݣ��򷵻ض�Ӧ��ָ�룬���򷵻� NULL
 */
Message * Net::Recv()
{
	if (m_recvQueue.empty()) {
		return NULL;
	}

	Message * msg = m_recvQueue.front();
	m_recvQueue.pop();
	return msg;
}

/*
 * ���� Message 
 */
void Net::Put(Message * msg)
{
	m_msgPool->Put(msg);
}

/*
 * ��ʼ��������
 */
bool Net::StartRecv()
{
	m_hRecvHandle = (HANDLE)_beginthreadex(NULL, 0, RecvThreadFunc, NULL, 0, NULL);
	if (m_hRecvHandle == INVALID_HANDLE_VALUE) {
		CCLOG("create recv thread failed, error = %ld\n", WSAGetLastError());
		return false;
	}
	return true;
}

/*
 * �������ݵĻص�����
 */
unsigned __stdcall Net::RecvThreadFunc(void* args)
{
	Message * msg = NULL;
	int nRead = 0;		// ���յ����ֽ���
	int nLast = 0;		// ʣ����յ��ֽ���
	char * buf = NULL;	// ���ջ�����
	bool ok = false;	// ����ֵ
	unsigned int msgLen = 0;
	unsigned int msgID = 0;

	while (!m_closed)
	{
		msgLen = 0;
		msgID = 0;

		// ����msgLen
		ok = Recv(m_connetSocket, (char*)&(msgLen), 4, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgLen error: %ld", SOCKET_ERROR);
				Close();// �ر�����
			}
			continue;
		}

		// ����msgID
		ok = Recv(m_connetSocket, (char*)&(msgID), 4, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgID error: %ld", SOCKET_ERROR);
				Close();// �ر�����
			}
			continue;
		}

		// ������Ϣ����data
		msg = m_msgPool->Get(msgLen);

		ok = Recv(m_connetSocket, (char *)msg->data, msgLen, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgID error: %ld", SOCKET_ERROR);
				m_msgPool->Put(msg);
				Close();// �ر�����
			}
			continue;
		}

		// ����Ϣ������Ϣ����
		msg->msgID = msgID;
		msg->msgLen = msgLen;
		m_recvQueue.push(msg);
	}

	return 0;
}

/*
 * ������Ϣ��������recv�Ľ�һ����װ
 *
 * ������
 *  s��   �׽���
 *  buf�� �������ݵĻ����������������ݽ�����Ϊֹ
 *  len�� ��������С
 */
bool Net::Recv(SOCKET s, char * buf, int len, int flags)
{
	int nRead = 0;
	int nLast = len;
	char * p = buf; // ����buf���׵�ַ

	while (nLast > 0) {
		nRead = recv(m_connetSocket, buf, nLast, 0);
		if (nRead == SOCKET_ERROR) {
			return false;
		}

		nLast -= nRead;
		buf += nRead;
	}

	buf = p;
	return true;
}

/*
 * �������������߳�
 */
bool Net::StartSend()
{
	m_hSendHandle = (HANDLE)_beginthreadex(NULL, 0, SendThreadFunc, NULL, 0, NULL);
	if (m_hSendHandle == INVALID_HANDLE_VALUE) {
		CCLOG("create send thread failed, error = %ld\n", WSAGetLastError());
		return false;
	}
	return true;
}

/*
 * ���������߳�
 */
unsigned __stdcall Net::SendThreadFunc(void* arg)
{
	Message * msg = NULL;
	int nSendSize = 0;
	int nLen = 0;
	char * buf = NULL;

	while (!m_closed) {
		if (m_sendQueue.empty()) {
			Sleep(50);
			continue;
		}
		
		msg = m_sendQueue.front();
		m_sendQueue.pop();
		
		buf = (char*)msg;
		nLen = msg->msgLen + 8;

		// ȷ��������������
		while (nLen > 0)
		{
			nSendSize = send(m_connetSocket, buf, nLen, 0);
			if (nSendSize == SOCKET_ERROR) {
				CCLOG("send failed with error: %d\n", WSAGetLastError());
				m_msgPool->Put(msg);
				Close();
				break;
			}

			nLen -= nSendSize;
			buf += nSendSize;
		}

		// ������Դ
		m_msgPool->Put(msg);
	}

	return 0;
}

void Net::Close()
{
	if (m_closed) {
		return;
	}

	CloseHandle(m_hRecvHandle);
	CloseHandle(m_hSendHandle);

	m_msgPool->Clear();

	m_closed = true;
	closesocket(m_connetSocket);
	WSACleanup();
}