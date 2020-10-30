#ifndef HMSERVER_H
#define HMSERVER_H

#include <QObject>
#include <QTcpServer>

class HmServer : public QObject
{
    Q_OBJECT
public:
    explicit HmServer(QObject *parent = nullptr);

    bool start();

signals:

public slots:
    void onNewConnection();

private:
    QTcpServer _server;
};

#endif // HMSERVER_H
