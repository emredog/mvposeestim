#ifndef CONVERTMODELTHREAD_H
#define CONVERTMODELTHREAD_H

#include <QThread>
#include <QDir>

class ConvertModelThread : public QThread
{
    Q_OBJECT
public:
    explicit ConvertModelThread(const int id, const QString exePath, const QDir dataDir, QObject *parent = 0);

signals:

public slots:
protected:
    void run();
    QDir dataDir;
    QString program;
    int id;

};

#endif // CONVERTMODELTHREAD_H
