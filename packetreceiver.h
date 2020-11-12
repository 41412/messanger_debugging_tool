#ifndef CHATPACKETRECEIVER_H
#define CHATPACKETRECEIVER_H

#include <QByteArray>
#include <QList>

class PacketReceiver
{
public:
    PacketReceiver();

    int process(const QByteArray& ba);
    QByteArray getPacket();
    int getPacketCount();

private:
    QList<QByteArray> packets;
    QByteArray buffer;
    qint32 remainSize;
};

#endif // CHATPACKETRECEIVER_H
