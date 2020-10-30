#ifndef HMCLIENT_H
#define HMCLIENT_H

#include <QObject>
#include <QTcpSocket>

class HmClient : public QObject
{
    Q_OBJECT
public:
    explicit HmClient(int id, QObject *parent = nullptr);

    bool connectTo(const QString url, unsigned int port);
    bool send(const QByteArray& data);
    bool isOpen();
    void disconnectFrom();
    void close();


public slots:
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);

signals:
    void onRead(const QByteArray& data);
    void socketStateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket  _socket;
    int _id;
};

#endif // HMCLIENT_H
