#include "tcpthread.h"

TcpThread::TcpThread(int socketDescriptor)
	: QThread(),socketDescriptor(socketDescriptor)
{
}

TcpThread::~TcpThread()
{
	//qDebug()<<socketDescriptor<<"~tcpThread end";
}

void TcpThread::run()
{
	/**
	* �ڴ�й©���
	*/
	//_CrtDumpMemoryLeaks();
	//
	/**
	*  ���ݿ����ӳز���
	*/
	//// �����ݿ����ӳ���ȡ������
	//QSqlDatabase db = ConnectionPool::openConnection();
	////qDebug() << "In thread run():" << db.connectionName();
	//QSqlQuery query(db);
	//query.exec("SELECT pwd FROM users where id=1");
	//while (query.next()) {
	//	qDebug() << query.value(0).toString();
	//}
	//// ����ʹ�������Ҫ�ͷŻ����ݿ����ӳ�
	//ConnectionPool::closeConnection(db);
	//
	TotalBytes = 0;  
	bytesReceived = 0;  
	fileNameSize = 0;  
	blockSize = 0;
	loadSize = 4*1024;
	bytesToWrite = 0;
	bytesWritten = 0;
	blFileOpen = false;
	blerror = false;
	upload_AND_download_Path = "G:\\TEEData\\";
	tcpServerConnection = new QTcpSocket;
	if (!tcpServerConnection->setSocketDescriptor(socketDescriptor)) {
		emit error(tcpServerConnection->error());
		return;
	}
	connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(receiveData()),Qt::DirectConnection);
	connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(displayError(QAbstractSocket::SocketError)), Qt::DirectConnection); 
	connect(tcpServerConnection, SIGNAL(connectionClosed()), this, SLOT(connectError()), Qt::DirectConnection);
	exec();
}

void TcpThread::connectError()
{
	//_qmutex.lock();
	//_datastore->insertSystrmErrorInfo("�Ͽ�tcp", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
	//_qmutex.unlock();
	if(blFileOpen)
	{
		localFile->close();
		localFile->remove(m_filePath + sFileName);
		localFile->deleteLater();
	}
	qDebug()<<"connectError";
	tcpServerConnection->deleteLater();
	emit disconnectedSignal(socketDescriptor);
	terminate();
}

void TcpThread::receiveData()    //�����ļ�
{
	try
	{
		QDataStream in(tcpServerConnection);  
		in.setVersion(QDataStream::Qt_4_7); 
		if(bytesReceived <= sizeof(qint64)*2)  
		{
			if((tcpServerConnection->bytesAvailable() >= sizeof(qint64) * 2) && (fileNameSize == 0))  
			{  
				in>>TotalBytes>>fileNameSize;  
				bytesReceived += sizeof(qint64)*2;  
			}  
			if((tcpServerConnection->bytesAvailable() >= fileNameSize) && (fileName == 0))  
			{ 
				in>>fileName;  
				dataProcess(fileName);
			}  
			else  
			{  
				tcpServerConnection->disconnect();
				tcpServerConnection->deleteLater();
				emit disconnectedSignal(socketDescriptor);
				quit();  
			} 
		}
		/*�ļ��ϴ�����*/
		if(sFile == "UP_FILE><UP_END")
		{
			if(bytesReceived<TotalBytes)  
			{  
				bytesReceived += tcpServerConnection->bytesAvailable();  
				inBlock = tcpServerConnection->readAll();  
				localFile->write(inBlock);  
				inBlock.resize(0);  
			}   
			if(bytesReceived == TotalBytes)  
			{   
				localFile->close();
				//m_mutexSql.lock();
				//QMutexLocker _locker(&m_mutexSql);
				if (!insertDataToSql(_username, m_filePath.replace('\\','/') + _currentFilename.replace('\\','/').left(_currentFilename.lastIndexOf('/')), 
					m_filePath + _currentFilename, TotalBytes, FileDigest(m_filePath + _currentFilename), "NULL"))
				{
					qDebug()<<_currentFilename + "·��д�����ݿ�ʧ�ܣ�";
				}
				//m_mutexSql.unlock();
				//qDebug()<<sFileName;
				TotalBytes = 0;
				bytesReceived = 0;
				fileNameSize = 0;
				m_filePath.clear();
				fileName.clear();
				sFileName.clear();
				m_serverPath.clear();
				sFile.clear();
				blFileOpen = false;
				tempPathstory.clear();
				currenttime.clear();
				tcpServerConnection->disconnect();
				tcpServerConnection->disconnectFromHost();
				tcpServerConnection->deleteLater();
				localFile->deleteLater();
				delete localFile;
				emit disconnectedSignal(socketDescriptor);
				quit();
			}
		}
		/*�����ļ�����*/
		else if(sFile == "DOWN_FILE><DOWN_END")
		{
			connect(tcpServerConnection,SIGNAL(bytesWritten(qint64)),this,SLOT(updateClientProgress(qint64)), Qt::DirectConnection);
		}
	}
	catch(QString err)
	{}
}

void TcpThread::dataProcess(QString _data)
{
	sFile = fileName.left(fileName.indexOf('>') + 1);
	sFile += fileName.right(fileName.size()- fileName.lastIndexOf('<'));
	sFileName = fileName.right(fileName.size() - fileName.lastIndexOf('>') - 1);
	sFileName = sFileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
	sFileName = sFileName.left(sFileName.indexOf('<'));
	m_serverPath = fileName.right(fileName.size() - fileName.indexOf('>') - 1);//m_serverPath  ������ʵ����
	m_serverPath = m_serverPath.left(m_serverPath.indexOf('<'));
	/*�����ļ�*/
	if(sFile == "DOWN_FILE><DOWN_END")
	{
		//���ҷ��������ļ��Ƿ����
		path = upload_AND_download_Path + m_serverPath.replace("/", "\\"); 
		qDebug()<<"path:"<<path;
		dir.setFile(path);
		bool blInfo = false;
		if(dir.exists())
		{
			blInfo = true;
		}
		if(!blInfo)
		{   //û��Ҫ���ص�����ļ�
			serverData = "no such file";
			serverMessage = "DATA_BEGIN>";
			serverMessage += serverData;
			serverMessage += "<DATA_END";
			QDataStream out(&block,QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_7);
			out<<qint64(0)<<qint64(0)<<serverMessage;
			TotalBytes += block.size();
			out.device()->seek(0);
			out<<TotalBytes<<qint64(block.size() - sizeof(qint64)*2);
			tcpServerConnection->write(block);
			if(!tcpServerConnection->waitForBytesWritten(5000))
			{
				qDebug()<<"data transfer error";
			}
			TotalBytes = 0;
			bytesReceived = 0;
			fileNameSize = 0;
			fileName.clear();
			sFileName.clear();
			sFile.clear();
			serverData.clear();
			serverMessage.clear();
			m_qfileinfolist.clear();
			m_serverPath.clear();
			tcpServerConnection->disconnect();
			tcpServerConnection->disconnectFromHost();
			tcpServerConnection->deleteLater();
			emit disconnectedSignal(socketDescriptor);
			quit();
		}
		fileName = path;
		localFile = new QFile(fileName);
		if(!localFile->open(QFile::ReadOnly))  
		{  
			blDownLoadFileOpen = false;
			localFile->deleteLater();
			return;  
		}  
		blDownLoadFileOpen = true;
		TotalBytes = localFile->size(); 
		QDataStream sendOut(&outBlock,QIODevice::WriteOnly);  
		sendOut.setVersion(QDataStream::Qt_4_7);  
		QString currentFile = fileName.right(fileName.size()-  
			fileName.lastIndexOf('\\')-1);        //.right��ȥ�ļ�·�����֣������ļ����ֱ�����currentFile��
		currentFile = sFileName;
		sendOut<<qint64(0)<<qint64(0)<<currentFile;  //����һ����ʱ���ļ�ͷ
		TotalBytes += outBlock.size();  //����ļ�ͷ��ʵ�ʴ洢��С
		sendOut.device()->seek(0);  //����д����ָ���ͷ��ʼ
		sendOut<<TotalBytes<<qint64(outBlock.size()- sizeof(qint64)*2);  
		bytesToWrite = TotalBytes - tcpServerConnection->write(outBlock);  
		qDebug()<<currentFile<<TotalBytes;  
		outBlock.resize(0);
	}
	/*�ϴ��ļ�*/
	else if(sFile == "UP_FILE><UP_END")     
	{
		m_serverPath = m_serverPath.replace('/',"\\");
		m_upFilePath = m_serverPath;
		currenttime = QDateTime::currentDateTime().toString("yyyyMMdd");
		bytesReceived += fileNameSize;  
		m_upFilePath = m_upFilePath.replace('/','\\');
		QDir qdircheck;
		_username = m_upFilePath.left(m_upFilePath.indexOf('\\'));/*�û���*/
		_currentFilename = m_upFilePath.right(m_upFilePath.size() - m_upFilePath.lastIndexOf('\\') - 1);
		m_filePath = upload_AND_download_Path + _username + "\\" + currenttime + 
			m_upFilePath.remove(m_upFilePath.left(m_upFilePath.indexOf('\\')));
		m_filePath = m_filePath.left(m_filePath.lastIndexOf('\\')) + "\\";
		if(!qdircheck.exists(m_filePath))
		{   //�ļ�������
			qdircheck.mkpath(m_filePath);
		}
		else if (qdircheck.exists(m_filePath))
		{   //�ļ�����
		}
		localFile = new QFile(m_filePath + _currentFilename);  //�ļ��洢��·��
		if(!localFile->open(QFile::WriteOnly))  
		{  
			blFileOpen = false;
			localFile->deleteLater();
			emit disconnectedSignal(socketDescriptor);
			quit();
		}  
		blFileOpen = true;
	}
	/*���Ͳ�ѯ�����ļ��б�(�����ļ�ϵͳ�Ĳ���)*/
	else if (sFile == "FILENAME_BEGIN><FILENAME_END")
	{
		/*��������*/
		/**
		*  ����
		*/
		//m_mutexSql.lock();
		//_datastore.testfun("123456"); 
		//m_mutexSql.unlock();
		QString _filter;
		m_qfileinfolist = GetFileList(upload_AND_download_Path + m_serverPath); 
		foreach(QFileInfo _fileinfo, m_qfileinfolist)
		{
			_filter = _fileinfo.completeSuffix();
			if(_filter != "avi")
			{
				continue;
			}
			if (!sFileName.isEmpty())
			{
				serverData.append(_fileinfo.absoluteFilePath().replace('/','\\').remove(upload_AND_download_Path) + "|");
			}
		}
		serverData = serverData.left(serverData.length() - 1);
		serverMessage = "FILENAME_BEGIN>";
		serverMessage += serverData;
		serverMessage += "<FILENAME_END";
		QDataStream out(&block,QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_7);
		out<<qint64(0)<<qint64(0)<<serverMessage;
		TotalBytes += block.size();
		out.device()->seek(0);
		out<<TotalBytes<<qint64(block.size() - sizeof(qint64)*2);
		tcpServerConnection->write(block);
		if(!tcpServerConnection->waitForBytesWritten(5000))
		{
			qDebug()<<"data transfer error";
		}
		/*if (serverData != "")
		{
		qDebug()<<serverData;
		}*/
		TotalBytes = 0;
		bytesReceived = 0;
		fileNameSize = 0;
		fileName.clear();
		sFileName.clear();
		sFile.clear();
		serverData.clear();
		serverMessage.clear();
		m_qfileinfolist.clear();
		m_serverPath.clear();
		_filter.clear();
		tcpServerConnection->disconnect();
		tcpServerConnection->deleteLater();
		emit disconnectedSignal(socketDescriptor);
		quit();
	}
	/*�û��������*/
	else if (sFile == "USER_FEEDBACK><USER_FEEDBACK")
	{
		try
		{
			m_mutexSql.lock();
			bool _blisfeedback = insertUserFeedback(m_serverPath.left(m_serverPath.indexOf('&')), m_serverPath, 
				tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
			m_mutexSql.unlock();
			if (!_blisfeedback)
			{
				throw QString("д���ݿ����");
			}
			TotalBytes = 0;
			bytesReceived = 0;
			fileNameSize = 0;
			fileName.clear();
			sFileName.clear();
			sFile.clear();
			m_serverPath.clear();
			tcpServerConnection->disconnect();
			tcpServerConnection->disconnectFromHost();
			tcpServerConnection->deleteLater();
			emit disconnectedSignal(socketDescriptor);
			quit();
		}
		catch(QString err)
		{
			TotalBytes = 0;
			bytesReceived = 0;
			fileNameSize = 0;
			fileName.clear();
			sFileName.clear();
			sFile.clear();
			m_serverPath.clear();
			tcpServerConnection->disconnect();
			tcpServerConnection->disconnectFromHost();
			tcpServerConnection->deleteLater();
			emit disconnectedSignal(socketDescriptor);
			quit();
		}
	}
	/*������������*/
	else if (sFile == ".1.><.1.") 
	{
		sendDataToClient(search_List_End(m_serverPath));
		emit disconnectedSignal(socketDescriptor);
		quit();
	}
	/*�û���½��֤*/
	else if (sFile == ".2.><.2.") 
	{
		QStringList userLoginInfo = m_serverPath.split(".CASIT.");
		QString _userName, _userPassword;   //�û���������
		if (userLoginInfo.length() >= 2)
		{
			_userName = userLoginInfo[0];
			_userPassword = userLoginInfo[1];
		}
		/*    ���ݿ�ƥ���û���Ϣ    */
		int _userInfoCheck = 0;    
		//��ѯ���ݿ���֤�û���������
		//m_mutexSql.lock();
		if (searchUserAndPwd(_userName, _userPassword))
		{
			_userInfoCheck = 1;
		}
		else
		{
			_userInfoCheck = 0;
		}
		//m_mutexSql.unlock();
		/*******************************************/
		if (_userInfoCheck == 1)
		{
			qDebug()<<"user:" + _userName + "\t" + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
		}
		sendUserLoginAndRegisterCheck(_userInfoCheck);           //0:��֤ʧ�ܣ�1:��֤�ɹ�
		emit disconnectedSignal(socketDescriptor);
		quit();
	}
	/*�Ƿ�����*/
	else
	{ 
		tcpServerConnection->disconnect();
		tcpServerConnection->deleteLater();
		emit disconnectedSignal(socketDescriptor);
		quit();
	}
}

void TcpThread::updateClientProgress(qint64 numBytes)  
{  
	try
	{
		bytesWritten += (int)numBytes;  
		if(bytesToWrite > 0)  
		{  
			if(bytesToWrite > 0)  
			{  
				outBlock = localFile->read(qMin(bytesToWrite,loadSize));  
				bytesToWrite -= (int)tcpServerConnection->write(outBlock);  
				outBlock.resize(0);  
			}  
			else  
			{  
				if(blDownLoadFileOpen)
				{
					localFile->close();  
				}
			}  
		}  
		if(bytesWritten == TotalBytes)
		{
			qDebug()<<path + "\tdownload ok";
			localFile->close();
			TotalBytes = 0;
			bytesReceived = 0;
			fileNameSize = 0;
			m_filePath.clear();
			fileName.clear();
			sFileName.clear();
			sFile.clear();
			m_serverPath.clear();
			string_list.clear();
			path.clear();
			blDownLoadFileOpen = false;
			tcpServerConnection->disconnect();
			tcpServerConnection->disconnectFromHost();
			tcpServerConnection->deleteLater();
			localFile->deleteLater();
			emit disconnectedSignal(socketDescriptor);
			quit();
		}
	}
	catch(...)
	{
		bool _blisfeedback = insertSystrmErrorInfo("д���ݴ���", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
		if(blDownLoadFileOpen)
		{
			localFile->deleteLater();
			delete localFile;
		}
		emit disconnectedSignal(socketDescriptor);
		quit();
		qDebug()<<"error message";
	}
} 

void TcpThread::displayError(QAbstractSocket::SocketError socketError)  
{  
	QString statestring;
	switch(socketError)
	{
	case QAbstractSocket::UnconnectedState : statestring="the socket is not connected";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::HostLookupState : statestring="the socket is performing a host name lookup";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::ConnectingState : statestring="the socket has started establishing a connection";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::ConnectedState : statestring="a connection is established";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::BoundState : statestring="the socket is bound to an address and port";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::ClosingState : statestring="the socket is about to close";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	case QAbstractSocket::ListeningState : statestring="listening state";
		//qDebug()<<tcpServerConnection->errorString();
		break;
	default: statestring="unknown state";
	}
	qDebug()<<"�쳣\t" + tcpServerConnection->peerAddress().toString() + "\t" + tcpServerConnection->peerName() + "\t" +
		tcpServerConnection->errorString() + "\t" + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
	//if(socketError == QTcpSocket::RemoteHostClosedError)  
	//{
	//	//return; 
	//}
	//_qmutex.lock();
	//bool _blisfeedback = _datastore->insertSystrmErrorInfo("д���ݴ���", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
	//_qmutex.unlock();
	if(blFileOpen)
	{
		localFile->close();
		if(!sFileName.isEmpty())
		{
			localFile->remove(m_filePath + sFileName);
			localFile->deleteLater();
			delete localFile;
		}
	}
	tcpServerConnection->disconnect();
	tcpServerConnection->disconnectFromHost();
	tcpServerConnection->deleteLater();
	emit disconnectedSignal(socketDescriptor);
	terminate();

} 

QFileInfoList TcpThread::GetFileList(QString path)
{
	QDir dir(path);
	QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for(int i = 0; i != folder_list.size(); i++)
	{
		QString name = folder_list.at(i).absoluteFilePath();
		QFileInfoList child_file_list = GetFileList(name);
		file_list.append(child_file_list);
	}
	return file_list;
}

QString TcpThread::FileDigest(QString FilePath)   //MD5��
{  
	QFile file(FilePath);
	QString macmd5 = "";
	if(file.exists(FilePath))
	{
		file.open(QIODevice::ReadOnly);
		QTextStream stream(&file);
		//stream.setCodec(code);//�����������
		QString txt="";
		txt=stream.readAll();
		QByteArray byte;
		byte = txt.toAscii();
		macmd5=QCryptographicHash::hash(byte,QCryptographicHash::Md5).toHex().constData();
	}
	return macmd5;
} 

void TcpThread::sendDataToClient(QString _currentData)
{
	QDataStream out(&block,QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out<<qint64(0)<<qint64(0)<<_currentData;
	TotalBytes += block.size();
	out.device()->seek(0);
	out<<TotalBytes<<qint64(block.size() - sizeof(qint64)*2);
	tcpServerConnection->write(block);
	if(!tcpServerConnection->waitForBytesWritten(10000))
	{
	    qDebug()<<"data transfer error";
	}
	TotalBytes = 0;
	bytesReceived = 0;
	fileNameSize = 0;
	fileName.clear();
	sFileName.clear();
	sFile.clear();
	serverData.clear();
	serverMessage.clear();
	m_qfileinfolist.clear();
	m_serverPath.clear();
	tcpServerConnection->disconnect();
	tcpServerConnection->disconnectFromHost();
	tcpServerConnection->deleteLater();
}

QString TcpThread::search_List_End(QString _patient_Name)
{
	//m_mutexSql.lock();
	//m_patientdata3 = searchPatientData(_patient_Name);
	//m_mutexSql.unlock();
	QString _searchData;
	//_searchData = m_patientdata3._patient_ID + "|" + m_patientdata3._local_path + "|" + m_patientdata3._timer;   
	//qDebug()<<_searchData;
	return _searchData;
}

bool TcpThread::insertDataToSql(QString _username, QString _server_path, QString _localfile,double _sieze, QString _md5, QString _timer)
{
	bool blexit = true;
	QSqlDatabase db = ConnectionPool::openConnection();
	QSqlQuery query(db);
	QString sql = "insert into medicaldata (username, server_path, local_path, size, md5, timer) values ('" + _username + "','" + _server_path + "','" + _localfile.replace("\\", "\\\\") + "','"  + QString::number(_sieze) + "','" + _md5 + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if(!query.exec(sql))
	{
		qDebug()<<"sql exec error " + db.connectionName()<< data_base.lastError();
		blexit = false;
	}
	ConnectionPool::closeConnection(db);
	if (blexit)
	{
		qDebug()<<"����" + _localfile + "�ɹ�";
	}
	return blexit;
}

bool TcpThread::insertUserFeedback(QString _user, QString _feedback, QString _ip_address, int _port)
{
	bool blexit = true;
	QSqlDatabase db = ConnectionPool::openConnection();
	QSqlQuery query(db);
	QString sql = "insert into user_feedback (user, feedback, ip_address, port, time) values ('" + _user + "','" + _feedback + "','" + _ip_address + "','"  + QString::number(_port) + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if (!query.exec(sql))
	{
		qDebug()<<data_base.lastError();
		blexit = false;
		qDebug()<<"�û������ύʧ�ܣ�";
	}
	ConnectionPool::closeConnection(db);
	if (blexit)
	{
		qDebug()<<_user + "������";
	}
	return true;
}

bool TcpThread::searchUserAndPwd(QString _username, QString _password)
{
	bool blexit = false;
	QSqlDatabase db = ConnectionPool::openConnection();
	QSqlQuery query(db);
	QString sql = "select count(*) from users where name = '" + _username + "' and pwd = '" + _password + "'";
	if (!query.exec(sql))
	{
		qDebug()<<"sql exec error"<<data_base.lastError();
	}
	query.next();
	if (query.value(0) == 0)
	{
		qDebug()<<"No user:"<<_username;
	}
	else
	{
		blexit = true;
	}
	ConnectionPool::closeConnection(db);
	return blexit;
}

QString TcpThread::searchInfo(QString _keyWord)
{
	QSqlDatabase db = ConnectionPool::openConnection();
	QSqlQuery query(db);
	//QString sql = "select path from case_patient where patient_id = (select hno from info where preope like '%" + _keyWord + "%')";
	QString sql = "SELECT c.patient_id, c.path FROM case_patient c WHERE c.patient_id IN (SELECT i.hno FROM info i WHERE i.preope LIKE '%" + _keyWord + "%')";
	if (!query.exec(sql))
	{
		qDebug()<<"sql exec error " + db.connectionName();
	}
	QString _result;
	while(query.next())
	{
		_result += "?" + query.value(1).toString();
	}
	_result = _result.remove(0,1);
	ConnectionPool::closeConnection(db);
	return _result;
}

bool TcpThread::insertSystrmErrorInfo(QString _systrmerror, QString _ip_address, int _port)
{
	bool blexit = true;
	QSqlDatabase db = ConnectionPool::openConnection();
	QSqlQuery query(db);
	QString sql = "insert into system_error (error_information, ip_address, port_address, time) values ('" + _systrmerror + "','" + _ip_address + "','"  + QString::number(_port) + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if (!query.exec(sql))
	{
		blexit = false;
	}
	ConnectionPool::closeConnection(db);
	return blexit;
}

void TcpThread::sendUserLoginAndRegisterCheck(int _check)
{
	QDataStream out(&block,QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out<<qint64(0)<<qint64(0)<<QString::number(_check);
	TotalBytes += block.size();
	out.device()->seek(0);
	out<<TotalBytes<<qint64(block.size() - sizeof(qint64)*2);
	tcpServerConnection->write(block);
	if(!tcpServerConnection->waitForBytesWritten(5000))
	{
		qDebug()<<"data transfer error";
	}
	TotalBytes = 0;
	bytesReceived = 0;
	fileNameSize = 0;
	fileName.clear();
	sFileName.clear();
	sFile.clear();
	serverData.clear();
	serverMessage.clear();
	m_qfileinfolist.clear();
	m_serverPath.clear();
	tcpServerConnection->disconnect();
	tcpServerConnection->deleteLater();
	
}