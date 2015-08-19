#ifndef CONNECTSOCKET_H
#define CONNECTSOCKET_H

#include <QTcpSocket>
#include <QtNetwork/QtNetwork>
#include <QtCore/QtCore>

class connectSocket : public QTcpSocket
{
	Q_OBJECT

public:
	connectSocket(int socketdescriptor);
	~connectSocket();
	void SetDescriptor(int des) {m_Descriptor = des;}
signals:
	void disconnectedSignal(int);
	private slots:
		void readMessage();
		void sendMessage();
		void disconnectSlot();
private:
	int m_Descriptor;
	QString m_string;
	qint64 m_BlockSize;
};

#endif // CONNECTSOCKET_H
