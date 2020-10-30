#include "hmclient.h"
#include <QHostAddress>

HmClient::HmClient(int id, QObject *parent)
    : QObject(parent)
    , _id(id)
{
    QObject::connect(&_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(&_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
}

bool HmClient::connectTo(const QString url, unsigned int port)
{
    if (!_socket.isOpen()) {
        _socket.connectToHost(QHostAddress(url), port);
        return true;
    }
    else {
        qDebug() << "client is already opened\n";
    }
    return false;
}

bool HmClient::send(const QByteArray& data)
{
    qint64 len = _socket.write(data, data.length());
    if (len == data.length()) {
        return true;
    }
    return false;
}

void HmClient::onReadyRead() {
    QByteArray data = _socket.readAll();
    onRead(data);
    //qDebug() << "Client[" << _id << "] get data:" << text;
}

bool HmClient::isOpen()
{
    return _socket.isOpen();
}

void HmClient::close()
{
    _socket.close();
}

void HmClient::disconnectFrom()
{
    _socket.disconnectFromHost();
}

void HmClient::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    emit socketStateChanged(socketState);
}
