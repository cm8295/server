#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent)
	: QTcpServer(parent)
{
	m_threadNum = 0;
}

TcpServer::~TcpServer()
{

}

void TcpServer::incomingConnection(int socketDescriptor)
{
	if (m_threadNum > 50)
	{
		return;
	}
	TcpThread *_tcpthread = new TcpThread(socketDescriptor);
	connect(_tcpthread,SIGNAL(finished()),_tcpthread,SLOT(deleteLater()));
	connect(_tcpthread,SIGNAL(disconnectedSignal(int)),this,SLOT(clientDisconnected(int)));
	_tcpthread->start();
	m_threadNum++;
}

void TcpServer::clientDisconnected(int descriptor)
{
	m_mutex.lock();
	if (m_threadNum > 0)
	{
		m_threadNum--;
	}
	
	//qDebug() << descriptor << "removed";
	m_mutex.unlock();
}