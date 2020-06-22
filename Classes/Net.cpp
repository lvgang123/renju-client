#include "Net.h"
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <process.h>

#include "cocos2d.h"
USING_NS_CC;

/*
 * 静态成员变量
 */
SOCKET Net::m_connetSocket = INVALID_SOCKET;
bool Net::m_closed = true; // 初始化套接字未连接，则其连接是关闭的。
HANDLE Net::m_hRecvHandle = INVALID_HANDLE_VALUE;
HANDLE Net::m_hSendHandle = INVALID_HANDLE_VALUE;
Pool * Net::m_msgPool = new Pool();
queue<Message*> Net::m_recvQueue;
queue<Message*> Net::m_sendQueue;

/*
 * 连接服务端
 *
 * 返回值：
 *  连接成功返回true，否则返回false。
 */
bool Net::Connect(const char* ip, short port)
{
	if (!m_closed) {
		// 套接字已初始化，不进行第二次连接
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
 * 开启发送数据与接收数据线程
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
 * 发送数据
 */
void Net::Send(unsigned int msgLen, unsigned int serviceID, unsigned int methodID, char * data)
{
	Message * msg = m_msgPool->Get(msgLen);
	msg->SetMsgLen(msgLen);
	msg->SetServiceID(serviceID);
	msg->SetMethodID(methodID);
	msg->SetData(data);

	// 放入发送队列中
	m_sendQueue.push(msg);
}

/*
 * 接收数据
 *
 * 返回值
 *  接收数据队列中有数据，则返回对应的指针，否则返回 NULL
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
 * 回收 Message 
 */
void Net::Put(Message * msg)
{
	m_msgPool->Put(msg);
}

/*
 * 开始接收数据
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
 * 接收数据的回调函数
 */
unsigned __stdcall Net::RecvThreadFunc(void* args)
{
	Message * msg = NULL;
	int nRead = 0;		// 接收到的字节数
	int nLast = 0;		// 剩余接收的字节数
	char * buf = NULL;	// 接收缓冲区
	bool ok = false;	// 返回值
	unsigned int msgLen = 0;
	unsigned int serviceID = 0;
	unsigned int methodID = 0;

	while (!m_closed)
	{
		msgLen = 0;
		serviceID = 0;
		methodID = 0;

		// 接收msgLen
		ok = Recv(m_connetSocket, (char*)&(msgLen), 4, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgLen error: %ld", SOCKET_ERROR);
				Close();// 关闭链接
			}
			continue;
		}

		// 接收serviceID
		ok = Recv(m_connetSocket, (char*)&(serviceID), 4, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgID error: %ld", SOCKET_ERROR);
				Close();// 关闭链接
			}
			continue;
		}

		// 接收methodID
		ok = Recv(m_connetSocket, (char*)&(methodID), 4, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgID error: %ld", SOCKET_ERROR);
				Close();// 关闭链接
			}
			continue;
		}

		// 接收消息主体data
		msg = m_msgPool->Get(msgLen);
		ok = Recv(m_connetSocket, (char *)msg->data, msgLen, 0);
		if (!ok) {
			if (WSAGetLastError() == SOCKET_ERROR) {
				CCLOG("Recv msgID error: %ld", SOCKET_ERROR);
				m_msgPool->Put(msg);
				Close();// 关闭网络
			}
			continue;
		}

		// 将消息放入消息队列
		msg->SetMsgLen(msgLen);
		msg->SetServiceID(serviceID);
		msg->SetMethodID(methodID);

		m_recvQueue.push(msg);
	}

	return 0;
}

/*
 * 接收消息函数，对recv的进一步封装
 *
 * 参数：
 *  s：   套接字
 *  buf： 接收数据的缓冲区，缓冲区数据接收完为止
 *  len： 缓冲区大小
 */
bool Net::Recv(SOCKET s, char * buf, int len, int flags)
{
	int nRead = 0;
	int nLast = len;
	char * p = buf; // 保存buf的首地址

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
 * 开启发送数据线程
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
 * 发送数据线程
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
		nLen = msg->msgLen + msg->GetMsgHeadBinaryCnt(); // 接收的数据字节：数据头字节+真正数据字节

		// 确保发送所有数据
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

		// 回收资源
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