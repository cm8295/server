#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent)
	: QTcpServer(parent)
{
	//m_Descriptor = 0;
	m_threadNum = 0;
}

TcpServer::~TcpServer()
{

}

void TcpServer::incomingConnection(int socketDescriptor)
{
	//m_Descriptor++;
	/*fun1*/
	//QTcpSocket *m_tcpsocket = nextPendingConnection();
	//qDebug() << m_tcpsocket->peerAddress().toString() << QString::number(m_tcpsocket->peerPort());
	if (m_threadNum > 50)
	{
		return;
	}
	TcpThread *_tcpthread = new TcpThread(socketDescriptor);
	connect(_tcpthread,SIGNAL(finished()),_tcpthread,SLOT(deleteLater()));
	//connect(_tcpthread,SIGNAL(finished()),_tcpthread,SLOT(quit()));
	connect(_tcpthread,SIGNAL(disconnectedSignal(int)),this,SLOT(clientDisconnected(int)));
	_tcpthread->start();
	/*fun2*/
	//connectSocket *_socket = new connectSocket(socketDescriptor);
	//connect(_socket,SIGNAL(disconnectedSignal(int)),this,SLOT(clientDisconnected(int)));
	//_socket->setSocketDescriptor(socketDescriptor);
	//qDebug()<<"m_Descriptor = "<<m_Descriptor;
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