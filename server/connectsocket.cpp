#include "connectsocket.h"

connectSocket::connectSocket(int socketdescriptor)
	: QTcpSocket()
{
	connect(this, SIGNAL(readyRead()),this,SLOT(readMessage()));
	connect(this, SIGNAL(disconnected()),this,SLOT(disconnectSlot()));
	qDebug()<<this<<"created";
	m_Descriptor = socketdescriptor;
}

connectSocket::~connectSocket()
{
	qDebug()<<this<<"deleted";
}

void connectSocket::readMessage()
{
	qDebug()<<"readMessage";
	QDataStream in(this);
	in.setVersion(QDataStream::Qt_4_7);
	if (m_BlockSize == 0)
	{
		if(bytesAvailable() < (int)sizeof(quint64))
		{
			return;
		}
		in>>m_BlockSize;
	}
	if (bytesAvailable() < m_BlockSize)
	{
		return;
	}
	in>>m_string;
	m_BlockSize = 0;
	sendMessage();
	//emit disconnected();
}

void connectSocket::sendMessage()
{

}

void connectSocket::disconnectSlot()
{
	
	emit disconnectedSignal(m_Descriptor);
}
