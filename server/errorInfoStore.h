/************************************************************************/
/*                  公共错误信息                                        */
/************************************************************************/
#pragma once
#include <QString>
#include <QStringList>
extern QStringList errorInfoList;
class errorInfoStore
{
public:
	errorInfoStore(void);
	~errorInfoStore(void);
protected:
	 void storeErrorInfoToTxt(QStringList);
private:
	QStringList getErrorInfo;
};

