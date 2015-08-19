/************************************************************************/
/*                  存储错误信息线程                                    */
/************************************************************************/
#ifndef TXTTHREAD_H
#define TXTTHREAD_H

#include <QThread>
#include "errorInfoStore.h"

class txtThread : public QThread
{
	Q_OBJECT

public:
	txtThread(QObject *parent);
	~txtThread();
	void run();
private:
	QStringList m_getErrorInfo;
};

#endif // TXTTHREAD_H
