#include "datastore.h"


datastore::datastore(void)
{
}


datastore::~datastore(void)
{
}

QSqlDatabase datastore::getdatabase()
{
	m_mutex.lock();
	return data_base;
	m_mutex.unlock();
}

bool datastore::dataBaseConnect()
{
	//if (QSqlDatabase::contains("teeair"))
	//{
	//	//data_base = QSqlDatabase::database("teeair");
	//}
	//else
	//{
	//	data_base = QSqlDatabase::addDatabase("QMYSQL");
	//	data_base.setHostName("localhost");
	//	data_base.setPort(3306);
	//	data_base.setDatabaseName("teeair");
	//	data_base.setUserName("root");
	//	data_base.setPassword("123456");
	//	//data_base.setConnectOptions("CLIENT_SSL=1;CLIENT_IGNORE_SPACE=1");//使用SSL  
	//	if(!data_base.open())
	//	{
	//		return false;
	//	}
	//	else
	//	{
	//		return true;
	//	}
	//}
	data_base = QSqlDatabase::addDatabase("QMYSQL");
	data_base.setHostName("localhost");
	data_base.setPort(3306);
	data_base.setDatabaseName("teeair");
	data_base.setUserName("root");
	data_base.setPassword("123456");
	//data_base.setConnectOptions("CLIENT_SSL=1;CLIENT_IGNORE_SPACE=1");//使用SSL  
	if(!data_base.open())
	{
		return false;
	}
	else
	{
		return true;
	}
}

QSqlDatabase datastore::sqlConnect()
{
	QSqlDatabase _database;
	_database = QSqlDatabase::addDatabase("QMYSQL");
	_database.setHostName("localhost");
	_database.setPort(3306);
	_database.setDatabaseName("teeair");
	_database.setUserName("root");
	_database.setPassword("123456");
	//data_base.setConnectOptions("CLIENT_SSL=1;CLIENT_IGNORE_SPACE=1");//使用SSL  
	if(!_database.open())
	{
		return _database;
	}
	else
	{
		return _database;
	}
}

void datastore::dataBaseClose()
{
	if (data_base.isOpen())
	{
		data_base.close();
	}
}

bool datastore::insertDataToSql(QString _username, QString _server_path, QString _localfile,double _sieze, QString _md5, QString _timer)
{
	//m_mutex.lock();
	//dataBaseConnect();
	QSqlQuery query(data_base);
	QString sql = "insert into medicaldata (username, server_path, local_path, size, md5, timer) values ('" + _username + "','" + _server_path + "','" + _localfile.replace("\\", "\\\\") + "','"  + QString::number(_sieze) + "','" + _md5 + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if(!query.exec(sql))
	{
		//data_base.close();
		//m_mutex.unlock();
		return false;
	}
	//data_base.close();
	//m_mutex.unlock();
	return true;
}

bool datastore::insertUserFeedback(QString _user, QString _feedback, QString _ip_address, int _port)
{
	//m_mutex.lock();
	//dataBaseConnect();
	QSqlQuery query(data_base);
	QString sql = "insert into user_feedback (user, feedback, ip_address, port, time) values ('" + _user + "','" + _feedback + "','" + _ip_address + "','"  + QString::number(_port) + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if (!query.exec(sql))
	{
		//data_base.close();
		m_mutex.unlock();
		return false;
	}
	//data_base.close();
	//m_mutex.unlock();
	return true;
}

bool datastore::insertSystrmErrorInfo(QString _systrmerror, QString _ip_address, int _port)
{
	//m_mutex.lock();
	//dataBaseConnect();
	QSqlQuery query(data_base);
	QString sql = "insert into system_error (error_information, ip_address, port_address, time) values ('" + _systrmerror + "','" + _ip_address + "','"  + QString::number(_port) + "','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "')";
	if (!query.exec(sql))
	{
		//data_base.close();
		//m_mutex.unlock();
		return false;
	}
	//data_base.close();
	//m_mutex.unlock();
	return true;
}

bool datastore::execsql(QString _sql)
{
	//m_mutex.lock();
	//dataBaseConnect();
	QSqlQuery query(data_base);
	if (!query.exec(_sql))
	{
		//data_base.close();
		//m_mutex.unlock();
		return false;
	}
	//data_base.close();
	//m_mutex.unlock();
	return true;
}

QString datastore::searchPatientDataName(QString _patientName)
{
	//m_mutex.lock();
	//dataBaseConnect();
	QSqlQuery query(data_base);
	QStringList patient_ID = searchPatientID(_patientName);
	QString sql;
	QString patient_data_path;
	foreach(QString _ID, patient_ID)
	{
		sql = "select patient_ID,local_path,timer from v_search where patient_ID = " + _ID + " and hno = " + _ID;
		if (!query.exec(sql))
		{
			//data_base.close();
			//m_mutex.unlock();
			return patient_data_path;
		}
		while(query.next())
		{
			patient_data_path.append(":" + query.value(0).toString());
		}
		patient_data_path.remove(0,1);
	}
	//data_base.close();
	//m_mutex.unlock();
	return patient_data_path;
}

QStringList datastore::searchPatientID(QString _patientName)
{
	//m_mutex.lock();
	QSqlQuery query(data_base);
	QString sql = "select hno from info where preope LIKE '%" + _patientName + "%'";
	QStringList patient_ID;
	if (!query.exec(sql))
	{
		//data_base.close();
		//m_mutex.unlock();
		return patient_ID;
	}
	while(query.next())
	{
		patient_ID.append(query.value(0).toString());
	}
	//data_base.close();
	//m_mutex.unlock();
	return patient_ID;
}

struct PATIENTDATA datastore::searchPatientData(QString _patientName)
{
	//m_mutex.lock();
	QSqlQuery query(data_base);
	QStringList patient_ID = searchPatientID(_patientName);
	QString sql;
	QString patient_data_path;
	foreach(QString _ID, patient_ID)
	{
		sql = "select patient_ID,local_path,timer from v_search where patient_ID = " + _ID + " and hno = " + _ID;
		if (!query.exec(sql))
		{
			//data_base.close();
			m_mutex.unlock();
			return m_patientdata1;
		}
		while(query.next())
		{
			m_patientdata1._patient_ID.append("?" + query.value(0).toString());
			m_patientdata1._local_path.append("?" + query.value(1).toString().replace('/','\\').remove(CURRENT_PATH));
			m_patientdata1._timer.append("?" + query.value(2).toString());
		}
		m_patientdata1._patient_ID.remove(0,1);
		m_patientdata1._local_path.remove(0,1);
		m_patientdata1._timer.remove(0,1);
	}
	//m_mutex.unlock();
	return m_patientdata1;
}

bool datastore::searchUserAndPwd(QString _username, QString _password)
{
	bool blexit = false;
	//QSqlDatabase _database = sqlConnect();
	QSqlQuery query(data_base);
	QString sql = "select count(*) from users where name = '" + _username + "' and pwd = '" + _password + "'";
	if (!query.exec(sql))
	{
		qDebug()<<"sql exec error"<<data_base.lastError();
	}
	query.next();
	if (query.value(0) == 0)
	{
		qDebug()<<"not exit a file name "<<_username;
	}
	else
	{
		blexit = true;
	}
	if (!blexit)
	{
		return false;
	}
	return true;
}

bool datastore::insertTestData(QString _tableName)
{
	QSqlQuery query(data_base);
	int _hno = 1123129;
	QString _preope = "心脏疾病";
	/*QString sql_info = "insert into info "
		"(aao, age, amv, anes, ao, aus, av, avd, bpd, bpm, "
		"cavitya, cavityb, dao, date, ecg, edd, edv, ef, emv, eoa, "
		"esd, esv, flapa, flapb, flowa, flowb, fs, gt, height, hno, "
		"hr, icu, ivs, la, lpa, lv, lvpw, mh, mpa, mva, "
		"mvo, name, ope, opesum, pasp, picsrc, picsrc2, picsrc3, preope, pv, "
		"ra, rpa, rv, sex, spo2, surgeon, sv, td, tr, ud, "
		"underope, walla, wallb, weight, worka,workb, xray) "
		"values ('0','0','0','0','0','0','0','0','0','0', '0','0','0','0','0','0','0','0','0','0', '0','0','0','0','0','0','0','0','0','0','"
		+ _hno + "','0','0','0','0','0','0','0','0','0','0', '0','0','0','0','0','0','0,'" 
		+ _preope + "','0','0', '0','0','0','0','0','0','0','0','0','0', '0','0','0','0','0','0','0')";*/
	QString sql_medicaldata = "insert into medicaldata (username, server_path, local_path, size, md5, timer, patient_ID) values ('张三','huaxi/ss001/ss0k3/a.avi','G:\\\\Teedata\\\\huaxi\\\\ss001\\\\ss0k3\\\\a.avi','33422','4445','" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "','" +QString::number(_hno + 1) + "')";
	QString sql_info ="INSERT INTO info (aao, age, amv, anes, ao, aus, av, avd, bpd, bpm, cavitya, cavityb, dao, DATE, ecg, edd, edv, ef, emv, eoa, esd, esv, flapa, flapb, flowa, flowb, fs, gt, height, hno, hr, icu, ivs, la, lpa, lv, lvpw, mh, mpa, mva, mvo, NAME, ope, opesum, pasp, picsrc, picsrc2, picsrc3, preope, pv, ra, rpa, rv, sex, spo2, surgeon, sv, td, tr, ud, underope, walla, wallb, weight, worka,workb, xray) "
		"VALUES ("
		"'0','0','0','0','0','0','0','0','0','0',"
		"'0','0','0','0','0','0','0','0','0','0',"
		"'0','0','0','0','0','0','0','0','0','" + QString::number(_hno + 1) + "',"
		"'0','0','0','0','0','0','0','0','0','0',"
		"'0','0','0','0','0','0','0','0','"+ _preope +"','0',"
		"'0','0','0','0','0','0','0','0','0','0',"
		"'0','0','0','0','0','0','0')" ;
	QString sql;
	for (int i = 0; i < 10; i++)
	{
		_hno += 1;
		sql = sql_medicaldata;
		if (!query.exec(sql))
		{
			return false;
		}
	}
	return true;
}