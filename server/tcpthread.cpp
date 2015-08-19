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
	* 内存泄漏检查
	*/
	//_CrtDumpMemoryLeaks();
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
	download_Path = "G:\\TEEData\\";
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
	//_datastore->insertSystrmErrorInfo("断开tcp", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
	//_qmutex.unlock();
	if(blFileOpen)
	{
		localFile->close();
		localFile->remove(m_filePath + sFileName);
		localFile->deleteLater();
		delete localFile;
	}
	qDebug()<<"connectError";
	tcpServerConnection->deleteLater();
	emit disconnectedSignal(socketDescriptor);
	terminate();
}

void TcpThread::receiveData()    //接收文件
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
				sFile = fileName.left(fileName.indexOf('>') + 1);
				sFile += fileName.right(fileName.size()- fileName.lastIndexOf('<'));
				sFileName = fileName.right(fileName.size() - fileName.lastIndexOf('>') - 1);
				sFileName = sFileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
				sFileName = sFileName.left(sFileName.indexOf('<'));
				m_serverPath = fileName.right(fileName.size() - fileName.indexOf('>') - 1);
				m_serverPath = m_serverPath.left(m_serverPath.indexOf('<'));
				/*下载文件*/
				if(sFile == "DOWN_FILE><DOWN_END")
				{
					//遍历文件夹
					path = download_Path + sFileName.replace("/", "\\"); 
					dir.setFile(path);
					bool blInfo = false;
					if(dir.exists())
					{
						blInfo = true;
					}
					if(!blInfo)
					{   //没有要下载的这个文件
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
						//delete localFile;
						return;  
					}  
					blDownLoadFileOpen = true;
					TotalBytes = localFile->size(); 
					QDataStream sendOut(&outBlock,QIODevice::WriteOnly);  
					sendOut.setVersion(QDataStream::Qt_4_7);  
					QString currentFile = fileName.right(fileName.size()-  
						fileName.lastIndexOf('\\')-1);        //.right出去文件路径部分，仅将文件部分保存在currentFile中
					currentFile = sFileName;
					sendOut<<qint64(0)<<qint64(0)<<currentFile;  //构造一个临时的文件头
					TotalBytes += outBlock.size();  //获得文件头的实际存储大小
					sendOut.device()->seek(0);  //将读写操作指向从头开始
					sendOut<<TotalBytes<<qint64(outBlock.size()- sizeof(qint64)*2);  
					bytesToWrite = TotalBytes - tcpServerConnection->write(outBlock);  
					qDebug()<<currentFile<<TotalBytes;  
					outBlock.resize(0);
				}
				/*上传文件*/
				else if(sFile == "UP_FILE><UP_END")     
				{
					currenttime = QDateTime::currentDateTime().toString("yyyyMMdd");
					bytesReceived += fileNameSize;  
					QDir qdircheck;
					_username = sFileName.left(sFileName.indexOf('\\'));/*用户名*/
					_currentFilename = sFileName.right(sFileName.size() - sFileName.lastIndexOf('\\') - 1);
					m_filePath = upload_AND_download_Path + _username + "\\" + currenttime + 
						sFileName.remove(sFileName.left(sFileName.indexOf('\\')));
					m_filePath = m_filePath.left(m_filePath.lastIndexOf('\\')) + "\\";
					if(!qdircheck.exists(m_filePath))
					{   //文件不存在
						qdircheck.mkpath(m_filePath);
					}
					else if (qdircheck.exists(m_filePath))
					{   //文件存在

					}
					localFile = new QFile(m_filePath + _currentFilename);  //文件存储的路径
					if(!localFile->open(QFile::WriteOnly))  
					{  
						blFileOpen = false;
						localFile->deleteLater();
						emit disconnectedSignal(socketDescriptor);
						quit();
					}  
					blFileOpen = true;
				}
				/*发送查询到的文件列表(基于文件系统的查找)*/
				else if (sFile == "FILENAME_BEGIN><FILENAME_END")
				{
					/*发送数据*/
					QString _filter;
					m_qfileinfolist = GetFileList(download_Path + sFileName); 
					foreach(QFileInfo _fileinfo, m_qfileinfolist)
					{
						_filter = _fileinfo.completeSuffix();
						if(_filter != "avi")
						{
							continue;
						}
						if (!sFileName.isEmpty())
						{
							serverData.append(_fileinfo.absoluteFilePath().replace('/','\\').remove(download_Path + sFileName.left(sFileName.indexOf('\\') + 1)) + "|");
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
					if (serverData != "")
					{
						qDebug()<<serverData;
					}
					emit disconnectedSignal(socketDescriptor);
					quit();
				}
				/*用户意见反馈*/
				else if (sFile == "USER_FEEDBACK><USER_FEEDBACK")
				{
					try
					{
						_qmutex.lock();
						bool _blisfeedback = _datastore.insertUserFeedback(m_serverPath.left(m_serverPath.indexOf('&')), m_serverPath, tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
						_qmutex.unlock();
						if (!_blisfeedback)
						{
							throw QString("写数据库错误");
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
						//wait();
					}
				}
				/*病例数据搜索*/
				else if (sFile == ".1.><.1.") 
				{
					sendDataToClient(search_List_End(m_serverPath));
					emit disconnectedSignal(socketDescriptor);
				}
				/*用户登陆验证*/
				else if (sFile == ".2.><.2.") 
				{
					QStringList userLoginInfo = m_serverPath.split(".CASIT.");
					QString _userName, _userPassword;   //用户名与密码
					if (userLoginInfo.length() >= 2)
					{
						_userName = userLoginInfo[0];
						_userPassword = userLoginInfo[1];
					}
					/*    数据库匹配用户信息    */
					int _userInfoCheck = 0;    
					//数据库查询
					QMutexLocker _qmutexlocker(&_qmutex);
					//_qmutex.lock();
					if (_datastore.searchUserAndPwd(_userName, _userPassword))
					{
						_userInfoCheck = 1;
					}
					else
					{
						_userInfoCheck = 0;
					}
					//_qmutex.unlock();
					/*******************************************/
					if (_userInfoCheck == 1)
					{
						qDebug()<<"user:" + _userName + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
					}
					sendUserLoginAndRegisterCheck(_userInfoCheck);           //0:验证失败，1:验证成功
				}
				/*非法连接*/
				else
				{ 
					tcpServerConnection->disconnect();
					tcpServerConnection->deleteLater();
					emit disconnectedSignal(socketDescriptor);
					quit();
				}
			}  
			else  
			{  
				tcpServerConnection->disconnect();
				tcpServerConnection->deleteLater();
				emit disconnectedSignal(socketDescriptor);
				quit();  
			} 
		}
		/*文件上传进度*/
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
				_qmutex.lock();
				_datastore.insertDataToSql(tempPathstory.left(tempPathstory.replace('/','\\').indexOf('\\')), m_serverPath, m_filePath + sFileName, TotalBytes, FileDigest(m_filePath + sFileName), "NULL");
				_qmutex.unlock();
				qDebug()<<sFileName;
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
				//sleep(3);
				tcpServerConnection->disconnect();
				tcpServerConnection->disconnectFromHost();
				tcpServerConnection->deleteLater();
				localFile->deleteLater();
				delete localFile;
				emit disconnectedSignal(socketDescriptor);
				quit();
			}
		}
		/*下载文件进度*/
		else if(sFile == "DOWN_FILE><DOWN_END")
		{
			connect(tcpServerConnection,SIGNAL(bytesWritten(qint64)),this,SLOT(updateClientProgress(qint64)), Qt::DirectConnection);
		}
	}
	catch(QString err)
	{}
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
			qDebug()<<"download ok";
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
			//sleep(3);
			tcpServerConnection->disconnect();
			tcpServerConnection->disconnectFromHost();
			tcpServerConnection->deleteLater();
			localFile->deleteLater();
			delete tcpServerConnection;
			delete localFile;
			emit disconnectedSignal(socketDescriptor);
			quit();
		}
	}
	catch(...)
	{
		_qmutex.lock();
		bool _blisfeedback = _datastore.insertSystrmErrorInfo("写数据错误", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
		_qmutex.unlock();
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
	qDebug()<<tcpServerConnection->peerAddress().toString() + tcpServerConnection->peerName()
		<<tcpServerConnection->errorString() + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
	//if(socketError == QTcpSocket::RemoteHostClosedError)  
	//{
	//	//return; 
	//}

	//_qmutex.lock();
	//bool _blisfeedback = _datastore->insertSystrmErrorInfo("写数据错误", tcpServerConnection->peerAddress().toString(), (int)tcpServerConnection->peerPort());
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

QString TcpThread::FileDigest(QString FilePath)   //MD5码
{  
	QFile file(FilePath);
	QString macmd5 = "";
	if(file.exists(FilePath))
	{
		file.open(QIODevice::ReadOnly);
		QTextStream stream(&file);
		//stream.setCodec(code);//输出流的设置
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
	//std::string str1 = _currentData.toStdString();
	//const char * ch = str1.c_str();
	//tcpServerConnection->write(ch);
	QDataStream out(&block,QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out<<qint64(0)<<qint64(0)<<_currentData;
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
	quit();
}

QString TcpThread::search_List_End(QString _patient_Name)
{
	m_patientdata3 = _datastore.searchPatientData(_patient_Name);
	QString _searchData = m_patientdata3._patient_ID + "|" + m_patientdata3._local_path + "|" + m_patientdata3._timer;   
	//qDebug()<<_searchData;
	return _searchData;
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
	emit disconnectedSignal(socketDescriptor);
	quit();
}
