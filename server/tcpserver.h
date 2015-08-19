#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "tcpthread.h"
#include "connectsocket.h"

class TcpServer : public QTcpServer
{
	Q_OBJECT

	void SetDescriptor(int des) {m_Descriptor = des;}
signals:
	void senddatatuthread(int _num);
	public slots:
		void clientDisconnected(int);
protected:
	void incomingConnection(int socketDescriptor);
public:
	TcpServer(QObject *parent);
	~TcpServer();
private:
	//QMap<int, connectSocket*> m_ClientDesMap;
	int m_Descriptor;//网络断开标识 1、0
	int m_threadNum;  //线程数
	QMutex m_mutex;
	//QTcpSocket *m_tcpsocket;
};

#endif // TCPSERVER_H
