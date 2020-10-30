#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>

class PacketParser : public QObject
{
    Q_OBJECT
public:
    explicit PacketParser(QObject *parent = nullptr);

    bool parse(const QByteArray& ba);
signals:


private:


};

#endif // PACKETPARSER_H
