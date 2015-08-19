#ifndef DATACHANGE_H
#define DATACHANGE_H

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QFileInfo>
//#include <QFileDialog>
#include <QTextCodec>
#include <QByteArray>
#include <QCryptographicHash>

class datachange : public QObject
{
	Q_OBJECT

public:
	datachange(QObject *parent);
	~datachange();
	QByteArray getFileMd5(QString filePath);   //获取文件MD5码
private:
	
};

#endif // DATACHANGE_H
