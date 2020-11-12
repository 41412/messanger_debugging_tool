#include "packetreceiver.h"
#include "packetparser.h"

PacketReceiver::PacketReceiver()
{

}
int PacketReceiver::process(const QByteArray& ba)
{
    bool validPacket = false;
    int sizePacket = 0;
    buffer += ba;

    while (buffer.size() != 0)
    {
        if (PacketParser::parseHeader(buffer, validPacket, sizePacket)) {
            int realSize = sizePacket + PacketParser::getHeaderSize();
            if (buffer.size() >= realSize) {
                packets += buffer.left(realSize);
                buffer.remove(0, realSize);
            }
            else {
                packets += buffer;
                remainSize = realSize - buffer.size();
                buffer.clear();
            }
        }
        else {
            if (remainSize > 0 && packets.size() > 0) {
                if (buffer.size() <= remainSize) {
                    packets.last() += buffer;
                    remainSize -= buffer.size();
                    buffer.clear();
                }
                else {
                    packets.last() += buffer.left(remainSize);
                    buffer.remove(0, remainSize);
                    remainSize = 0;
                }
            }
            else {
                qDebug("has wrong data");
                buffer.clear();
            }
        }
    }

    if(remainSize > 0) {
        return packets.size()-1;
    }
    return packets.size();
}

QByteArray PacketReceiver:: getPacket()
{
    QByteArray a;
    if (!packets.isEmpty()) {
        a = packets.front();
        packets.pop_front();
    }
    return a;
}

int PacketReceiver::getPacketCount()
{
    return packets.count();
}
