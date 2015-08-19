#pragma once
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>
#include <QStringList>
#include <QMutex>
#include <QSqlError>
#define CURRENT_PATH "G:\\TEEData\\Huaxi\\"
struct PATIENTDATA{
	QString _patient_ID;   //病人id，住院号
	QString _local_path;   //病例图像在本地路径
	QString _timer;       //图像传输的时间
};

class datastore
{
public:
	datastore(void);
	~datastore(void);
	bool dataBaseConnect();      //连接数据库
	QSqlDatabase sqlConnect();  //连接数据库
	void dataBaseClose();        //关闭数据流连接
	bool execsql(QString _sql);
	bool insertDataToSql(QString _username, QString _server_path, QString _localfile,double _sieze, QString _md5, QString _timer);
	bool insertUserFeedback(QString _user, QString _feedback, QString _ip_address, int _port);
	bool insertSystrmErrorInfo(QString _systrmerror, QString _ip_address, int _port);
	QString searchPatientDataName(QString _patientName);//在数据库中查询病例数据
	QStringList searchPatientID(QString _patientName);   //获取满足条件的病人住院号
	QSqlDatabase getdatabase();
	bool searchUserAndPwd(QString _username, QString _password);  //查询用户名和密码
	PATIENTDATA searchPatientData(QString _patientName);   //搜索病例数据
	bool insertTestData(QString _tableName);    //插入数据测试
private:
	QSqlDatabase data_base;
	struct PATIENTDATA m_patientdata1;
	//QString upload_AND_download_Path;
	QMutex m_mutex;
};

