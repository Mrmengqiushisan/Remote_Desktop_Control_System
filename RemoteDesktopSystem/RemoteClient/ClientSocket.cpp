#include "pch.h"
#include "ClientSocket.h"
CClientSocket::CHelper CClientSocket::m_helper;

void CClientSocket::threadEntry(void* arg){
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClientSocket::threadFunc(){
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index{ 0 };
	InitSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			CPacket& head = m_lstSend.front();
			if (Send(head) == false) {
				TRACE("����ʧ�ܣ�\r\n");
				continue;
			}
			auto it = m_mapAck.find(head.hEvent);
			auto it0 = m_mapAutoClosed.find(head.hEvent);
			do {
				int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
				if (length > 0 || index > 0) {
					index += length;
					size_t size = (size_t)index;
					CPacket pack((BYTE*)pBuffer, size);
					if (size > 0) {
						//TODO:�����ļ�����Ϣ��ȡ���ļ���Ϣ���ܻ��������
						pack.hEvent = head.hEvent;
						it->second.push_back(pack);
						memmove(pBuffer, pBuffer + size, index - size);
						index -= size;
						if (it0->second)SetEvent(head.hEvent);
					}
				}
				else if (length <= 0 && index <= 0) {
					CloseSocket();
					SetEvent(head.hEvent);//�ȵ��������ر�����֮��֪ͨ�������
				}
			} while (it0->second == false);
			m_lstSend.pop_front();
			InitSocket();
		}
	}
}
