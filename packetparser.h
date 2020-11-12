#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>
#include <QMap>
#include <QByteArray>

enum ServerMsg{
    LOGIN_SUCCESS = 0,
    LOGIN_FAIL,
    SUBMIT_SUCCESS,
    SUBMIT_FAIL,
    USERDATA_SEND_START,
    SEND_FRIENDLIST,
    SEND_CHATROOMLIST,
    USERDATA_SEND_END,
    RESPONSE_CREATE_CHATROOM,
    RESPONSE_INVITE_USER,
    RESPONSE_LEAVE_USER,
    UPDATE_CHAT,
    SERVER_MSG_MAX,
};

class PacketParser : public QObject
{
    Q_OBJECT
public:
    explicit PacketParser(QObject *parent = nullptr);

    QMap<QString,QByteArray> parse(const QByteArray& ba);

    static QByteArray makePacket(const QString& cmd, const QMap<QString, QByteArray>& vars);
    static bool parseHeader(const QByteArray& ba, bool& valid, int& packetSize);
    static int getHeaderSize();

signals:

private:


};

#endif // PACKETPARSER_H
