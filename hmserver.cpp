#include "hmserver.h"

HmServer::HmServer(QObject *parent) : QObject(parent)
{

}

bool HmServer::start()
{
    //_server.listen(QHostAddress::Any, 4242);
    //connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    return true;
}

void HmServer::onNewConnection()
{
    // QTcpSocket *clientSocket = _server.nextPendingConnection();
    // qDebug("A new connection!!!");
    // for (QTcpSocket* socket : _sockets) {
    //    socket->write(QByteArray::fromStdString(clientSocket->peerAddress().toString().toStdString() + " connected to server !\n"));
    //}
}
