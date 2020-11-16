#include "packetparser.h"
#include <QDataStream>
#include <string>
#include <QDebug>


#define LEVEL1_DELI (0x1D)
#define LEVEL2_DELI (0x1E)
#define LEVEL3_DELI (0x1F)

static const char signature[8]={'\xFF','\xEE','M','R','2','0','2','0'};

static int byteArrayToInt(const QByteArray& arr)
{
    QByteArray baSize = arr.left(4);
    QDataStream ds(&baSize,QIODevice::ReadOnly);
    qint32 size=0;
    ds >> size;
    return size;
}

static QByteArray intToArray(int i)
{
    QByteArray ba;
    QDataStream ds(&ba,QIODevice::ReadWrite);
    ds << i;
    return ba;
}

PacketParser::PacketParser(QObject *parent) : QObject(parent)
{

}


QByteArray extractProtocol(const QByteArray& message)
{
    int i = message.indexOf(0x1D);
    if (i >= 0) {
        return message.left(i);
    }
    else {
        return message;
    }
}

static inline QByteArray fillHeader(const QByteArray& ar)
{
    QByteArray b;
    b.append(signature, 8);
    b.append(intToArray(ar.length()));
    b += ar;
    return b;
}

static QMap<QString,QByteArray> parseClient(const QByteArray& ba)
{
    QMap<QString,QByteArray> m;
    bool valid = false;
    int size = 0;

//    struct CmdTable {
//        const char* cmd;
//        int id;
//    };

//    const static CmdTable table[SERVER_MSG_MAX] = {
//        { "LOGIN_SUCCESS", LOGIN_SUCCESS },
//        { "LOGIN_FAIL", LOGIN_FAIL },
//        { "SUBMIT_SUCCESS", SUBMIT_SUCCESS },
//        { "SUBMIT_FAIL", SUBMIT_FAIL },
//        { "USERDATA_SEND_START", USERDATA_SEND_START },
//        { "SEND_FRIENDLIST", SEND_FRIENDLIST },
//        { "SEND_CHATROOMLIST, SEND_CHATROOMLIST },
//        { "USERDATA_SEND_END", USERDATA_SEND_END },
//        { "RESPONSE_CREATE_CHATROOM", RESPONSE_CREATE_CHATROOM },
//        { "RESPONSE_INVITE_USER", RESPONSE_INVITE_USER },
//        { "RESPONSE_LEAVE_USER", RESPONSE_LEAVE_USER },
//        { "UPDATE_CHAT", UPDATE_CHAT }
//    };

//    static QMap<const char*, int> mt;
//    if (mt.size() == 0) {
//        for (int i = 0; i < SERVER_MSG_MAX; i++) {
//            mt[table[i].cmd] = table[i].id;
//        }
//    }

    if (PacketParser::parseHeader(ba, valid, size)) {
        QByteArray body = ba.mid(PacketParser::getHeaderSize());
        QByteArray cmd = extractProtocol(body);
        m["target"] = "client";

        if (cmd == "LOGIN_SUCCESS" || cmd == "LOGIN_FAIL") {
            m["cmd"] = "login";
            if (cmd == "LOGIN_SUCCESS")
                m["success"] = "true";
            else
                m["success"] = "false";
        }
        else if (cmd == "SUBMIT_SUCCESS" || cmd == "SUBMIT_FAIL") {
            m["cmd"] = "register";
            if (cmd == "SUBMIT_SUCCESS")
                m["success"] = "true";
            else
                m["success"] = "false";
        }
        else if (cmd == "USERDATA_SEND_START") {
            m["cmd"] = "userdata.start.send";
        }
        else if (cmd == "SEND_FRIENDLIST") {
            QList<QByteArray> list = body.split(LEVEL1_DELI);
            if (list.size() > 2) {
                m["cmd"] = "send.friend.list";
                m["count"] = list.at(1);
                int i = body.indexOf(list.at(2));
                m["members"] = body.mid(i);
            }
            else {
                qDebug() << "invalid friend list";
            }
        }
        else if (cmd == "SEND_CHATROOMLIST") {
            QList<QByteArray> list = body.split(LEVEL1_DELI);
            if (list.size() > 2) {
                m["cmd"] = "send.room.list";
                m["count"] = list.at(1);
                int count = m["count"].toInt();
                for (int n = 0; n < count; n++) {
                    QString roomNum = QString::asprintf("room%d", n);
                    m[roomNum] = list[n+2].replace(LEVEL2_DELI, ' ');
                }
            }
            else {
                qDebug() << "invalid friend list";
            }
        }
        else if (cmd == "USERDATA_SEND_END") {
            m["cmd"] = "userdata.end.send";
        }
        else if (cmd == "RESPONSE_CREATE_CHATROOM") {
            QList<QByteArray> list = body.split(LEVEL1_DELI);
            m["cmd"] = "created.room";
            if (list.size() > 1) {
                m["roomid"] = list.at(1);
            }
        }
        else if (cmd == "RESPONSE_INVITE_USER") {

        }
        else if (cmd == "RESPONSE_LEAVE_USER") {

        }
        else if (cmd == "UPDATE_CHAT") {
            QList<QByteArray> list = body.split(LEVEL1_DELI);
            if (list.size() > 5) {
                m["cmd"] = "update.chat";
                m["roomid"] = list.at(1);
                m["name"] = list.at(2);
                m["time"] = list.at(3);
                m["index"] = list.at(4);
                m["msg"] = list.at(5);
            }
            else {
                qDebug("invalid chat packet");
            }
        }
    }
    else {
        qDebug("invalid packet");
    }

    return m;
}

QMap<QString,QByteArray> PacketParser::parse(const QByteArray& ba)
{
    // now test for client
    return parseClient(ba);
}

bool PacketParser::parseHeader(const QByteArray& ba, bool& valid, int& size)
{
    valid = false;
    size = 0;
    if (memcmp(ba.constData(), signature, 8) == 0) {
        valid = true;
        size = byteArrayToInt(ba.mid(8,4));
        return true;
    }
    return true;
}

int PacketParser::getHeaderSize()
{
    return 12;
}

QByteArray PacketParser::makePacket(const QString& cmd, const QMap<QString, QByteArray>& vars)
{
    QByteArray ba;

    if (vars["target"] == "client") {
        if (cmd == "req.login") {
            ba += "REQUEST_LOGIN";
            ba += LEVEL1_DELI;
            ba += vars["name"];
            ba += LEVEL1_DELI;
            ba += vars["password"];
            return fillHeader(ba);
        }
        else if (cmd == "req.register") {
            ba += "REQUEST_SUBMIT";
            ba += LEVEL1_DELI;
            ba += vars["name"];
            ba += LEVEL1_DELI;
            ba += vars["password"];
            return fillHeader(ba);
        }
        else if (cmd == "req.user.data") {
            ba += "REQUEST_USERDATA";
            ba += LEVEL1_DELI;
            ba += vars["name"];
            return fillHeader(ba);
        }
        else if (cmd == "notify.ready.receive") {
            ba += "READY_TO_RECEIVE";
            return fillHeader(ba);
        }
        else if (cmd == "res.received.friend.list") {
            ba += "FRIENDLIST_RECEIVED";
            return fillHeader(ba);
        }
        else if (cmd == "res.received.room.list") {
            ba += "CHATROOMLIST_RECEIVED";
            return fillHeader(ba);
        }
        else if (cmd == "req.create.room") {
            ba += "REQUEST_CREATE_CHATROOM";
            ba += LEVEL1_DELI;
            ba += vars["name"];
            ba += LEVEL1_DELI;
            ba += vars["count"];
            ba += LEVEL1_DELI;
            QByteArray members = vars["members"];
            ba += members.replace(' ', LEVEL1_DELI);
            return fillHeader(ba);
        }
        else if (cmd == "req.invite.user") {
            ba += "REQUEST_INVITE_USER";
            ba += LEVEL1_DELI;
            ba += vars["roomid"];
            ba += LEVEL1_DELI;
            QByteArray members = vars["members"];
            ba += members.replace(' ', LEVEL1_DELI);
            return fillHeader(ba);
        }
        else if (cmd == "req.leave.room") {
            ba += "REQUEST_LEAVE_CHATROOM";
            ba += LEVEL1_DELI;
            ba += vars["roomid"];
            ba += LEVEL1_DELI;
            ba += vars["name"];
            return fillHeader(ba);
        }
        else if (cmd == "req.send.chat") {
            ba += "REQUEST_SEND_CHAT";
            ba += LEVEL1_DELI;
            ba += vars["roomid"];
            ba += LEVEL1_DELI;
            ba += vars["name"];
            ba += LEVEL1_DELI;
            ba += vars["msg"];
            return fillHeader(ba);
        }
    }
    else {

    }

    return ba;
}
