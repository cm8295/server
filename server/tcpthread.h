#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#define CRTDBG_MAP_ALLOC//内存错误跟踪
#include <stdlib.h>
#include <crtdbg.h>

#include <QThread>
#include <QTcpSocket>
#include <QtNetwork/QHostAddress>
#include "tcpserver.h"
#include "datachange.h"
#include "datastore.h"
#include <QtSql/QSqlDatabase>
#include <QtXml/QtXml> 
#include <QDomDocument>  
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

enum DATA_FORMAT
{
	DOWN_FILEDOWN_END = 1,
	UP_FILEUP_END = 2,
	FILENAME_BEGINFILENAME_END = 3,
	SEARCH_PATIENT_DATA = 4         //数据搜索接口
};

class TcpThread : public QThread
{
	Q_OBJECT

public:
	TcpThread(int socketDescriptor);
	~TcpThread();
	void run();
	QFileInfoList GetFileList(QString path);   //获取文件列表
	QString FileDigest(QString FilePath);   //获取文件MD5码
	double fileSize(QString _filepath);
	QString search_List_End(QString _patient_Name);   //获取该病例名称下所有病例数据
	void sendDataToClient(QString _currentData);/*发送数据*/
	void sendUserLoginAndRegisterCheck(int _check);//用户信息检测结果
signals:
	void error(QTcpSocket::SocketError socketError);
	void bytesArrived(qint64,qint32,int);
	void sendtransferinfo();
	void disconnectedSignal(int);
	//void connectDis();
	public slots:
		void receiveData();  //接收客户端发过来的文件数据
		void updateClientProgress(qint64 numBytes);
		void displayError(QAbstractSocket::SocketError socketError);  
		void connectError();   //连接断开，终止本线程
private:
	/*公用*/
	int socketDescriptor;
	QTcpSocket *tcpServerConnection;  
	/**/
	qint64 TotalBytes;     //file总大小
	qint64 bytesReceived;  //收到的总字节
	qint64 fileNameSize;  //准备读取的字节
	QString fileName;  //文件路径名称
	QFile *localFile;  
	QFile m_localFile;
	QByteArray inBlock;
	QByteArray outBlock;
	qint64 bytesToWrite;
	qint64 bytesWritten;
	qint64 loadSize;
	//接收数据
	QString message;  //存放从服务器接收到的字符串  
	quint16 blockSize;  //存放文件的大小信息 
	QFileInfoList m_qfileinfolist;
	QString sFileName;    //真实文件名
	QString sFile;      //文件类型标识,上传或者下载
	QString m_filePath;  //文件存储路径
	QString m_serverPath;//文件的服务路径
	QString clientip;    //客户端ip
	QHostAddress qip;
	bool blFileOpen;
	bool blDownLoadFileOpen;
	QString currenttime;
	QString upload_AND_download_Path;
	QString download_Path;
	//datachange *_datachange;
	bool blerror;
	QString serverMessage;  
	QString serverData;
	QByteArray block;
	//download
	QStringList string_list;
	QString path;
	QFileInfo dir;
	//upload
	QFileInfo qfilecheck;
	QString tempPathstory;
	QString _username;/*用户名*/
	QString _currentFilename ;/*文件名*/

	//database
	QSqlDatabase data_base;
	datastore _datastore;
	QMutex _qmutex;
	struct PATIENTDATA m_patientdata3;    
};

#endif // TCPTHREAD_H
