/************************************************************************/
/*                     病例库服务器入口                                 */
/************************************************************************/
#include <QtCore/QCoreApplication>
#include <QTextCodec>
#include <tcpserver.h>
#include <Windows.h>
#include <DbgHelp.h>
#include "datastore.h"
#pragma comment( lib, "Dbghelp.lib" )
struct PATIENTDATA1{/*病人信息结构体*/
	QString _patient_ID;
	QString _local_path;
	QString _timer;
};

TcpServer *tcpServer;
QSqlDatabase data_base;

/**
*  打开服务器
*/
bool openServer()
{
	while (!tcpServer->isListening() && !tcpServer->listen(QHostAddress::Any,19845))
	{
		qDebug()<<tcpServer->errorString();
		return false;
	}
	tcpServer->setMaxPendingConnections(100);
	qDebug()<<"服务已经启动！"<<" 端口："<<19845;
	return true;
}
/**
*程序异常处理
*/
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException){//程式异常捕获  
    /* 
      ***保存数据代码*** 
    */  
	//创建 Dump 文件  
	HANDLE hDumpFile = CreateFile(QTime::currentTime().toString("HH时mm分ss秒zzz.dmp").utf16(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
	if( hDumpFile != INVALID_HANDLE_VALUE){  
		//Dump信息  
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;  
		dumpInfo.ExceptionPointers = pException;  
		dumpInfo.ThreadId = GetCurrentThreadId();  
		dumpInfo.ClientPointers = TRUE;  
		//写入Dump文件内容  
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);  
	}  
    //这里弹出一个错误对话框并退出程序  
    EXCEPTION_RECORD* record = pException->ExceptionRecord;  
    QString errCode(QString::number(record->ExceptionCode,16)),errAdr(QString::number((uint)record->ExceptionAddress,16)),errMod;  
	qDebug()<<"错误代码：" + errCode<<"错误地址：" + errAdr;
    return EXCEPTION_EXECUTE_HANDLER;  
} 

int main(int argc, char *argv[])
{
	//_CrtSetBreakAlloc(3023);
	QCoreApplication a(argc, argv);
	//获取系统编码，否则移植会出现乱码
	QTextCodec *codec = QTextCodec::codecForName("System"); 
	//设置和对本地文件系统读写时候的默认编码格式
	QTextCodec::setCodecForLocale(codec); 
	//用在字符常量或者QByteArray构造QString对象时使用的一种编码方式
	QTextCodec::setCodecForCStrings(codec); 
	//设置传给tr函数时的默认字符串编码
	QTextCodec::setCodecForTr(codec);
	//SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注册异常捕获函数
	//datastore *_datastore = new datastore(NULL);
	datastore _datastore;
	tcpServer = new TcpServer(NULL);
	openServer();
	if(_datastore.dataBaseConnect())
	{
		qDebug()<<"数据库链接成功";
		//_datastore.dataBaseClose();
	}
	else
	{
		qDebug()<<"数据库链接失败";
	}
	return a.exec();
}


