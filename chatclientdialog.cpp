#include "chatclientdialog.h"
#include "ui_chatclientdialog.h"
#include "packetparser.h"
#include "packetreceiver.h"

ChatClientDialog::ChatClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatClientDialog),
    _parser(new PacketParser),
    _receiver(new PacketReceiver),
    _socket()
{
    ui->setupUi(this);

    connect(&_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    //_socket.connectToHost("127.0.0.1", 35000);
}

ChatClientDialog::~ChatClientDialog()
{
    _socket.disconnectFromHost();

    delete ui;
}

void ChatClientDialog::on_pushButton_Login_clicked()
{
    QMap<QString, QByteArray> vars;

//    if (vars["target"] == "client") {
//        if (cmd == "req.login") {
//            ba += "REQUEST_LOGIN";
//            ba += LEVEL1_DELI;
//            ba += vars["name"];
//            ba += LEVEL1_DELI;
//            ba += vars["password"];
//            return fillHeader(ba);
//        }
    if (_socket.isOpen()) {
        vars["target"] = "client";
        vars["name"] = ui->lineEdit_Name->text().toUtf8();
        vars["password"] = ui->lineEdit_Password->text().toUtf8();
        QByteArray ba = PacketParser::makePacket("req.login", vars);
        _socket.write(ba);
    }
}

void ChatClientDialog::onReadyRead()
{
    int packetCount = _receiver->process(_socket.readAll());
    while (packetCount--) {
        QMap<QString,QByteArray> m = _parser->parse(_receiver->getPacket());
        if (m.contains("cmd")) {
            if (m["cmd"] == "login") {
                if (m["success"] == "true")
                    ui->listWidget_Received->addItem("login succeed");
                else
                    ui->listWidget_Received->addItem("login failed");
            }
            else if (m["cmd"] == "register") {
                if (m["success"] == "true")
                    ui->listWidget_Received->addItem("register succeed");
                else
                    ui->listWidget_Received->addItem("register failed");
            }
            else if (m["cmd"] == "userdata.start.send") {
                ui->listWidget_Received->addItem("-------- start user data --------");
            }
            else if (m["cmd"] == "send.friend.list") {
                QString item;
                item += "You have ";
                item += m["count"];
                item += " friends.";
                item += " ";
                item += m["members"].replace(0x1D, ' ');
                ui->listWidget_Received->addItem(item);
            }
            else if (m["cmd"] == "send.room.list") {
                 QString strCount = m["count"];
                 int count = strCount.toInt();
                 ui->listWidget_Rooms->clear();
                 for (int n = 0; n < count; n++) {
                     QString item;
                     QString roomNum = QString::asprintf("room%d", n);
                     QList<QByteArray> items = m[roomNum].split(' ');
                     if (items.size()) {
                         ui->listWidget_Rooms->addItem(items[0]);
                     }
                     item += m[roomNum].replace(0x1E, ' ');
                     ui->listWidget_Received->addItem(item);
                 }
            }
            else if (m["cmd"] == "userdata.end.send") {
                ui->listWidget_Received->addItem("-------- end user data --------");
            }
            else if (m["cmd"] == "created.room") {
                QString item;
                item += "Created ";
                item += m["roomid"];
                ui->listWidget_Received->addItem(item);
            }
            else if (m["cmd"] == "update.chat") {
                QString item;
                item += "[";
                item += m["roomid"];
                item += "]";
                item += m["time"];
                item += " ";
                item += m["index"];
                item += " ";
                item += m["msg"];

                ui->listWidget_Received->addItem(item);
            }
        }
    }
}

void ChatClientDialog::on_pushButton_Connect_clicked()
{
    if (_socket.isOpen()) {
        _socket.disconnectFromHost();
    }
    _socket.abort();
    _socket.connectToHost(ui->lineEdit_Address->text(), 35000);
}

void ChatClientDialog::on_pushButton_Disconnect_clicked()
{
    if (_socket.isOpen()) {
        _socket.disconnectFromHost();
    }
}

void ChatClientDialog::on_pushButton_Send_clicked()
{
    QMap<QString, QByteArray> vars;

    vars["target"] = "client";
    vars["roomid"] = ui->textEdit_RoomSelected->toPlainText().toUtf8();
    vars["name"] = ui->lineEdit_Name->text().toUtf8();
    vars["msg"] = ui->lineEdit_Input->text().toUtf8();

    _socket.write(PacketParser::makePacket("req.send.chat", vars));
}

void ChatClientDialog::on_listWidget_Rooms_itemClicked(QListWidgetItem *item)
{
    ui->textEdit_RoomSelected->setText(item->text());
}
