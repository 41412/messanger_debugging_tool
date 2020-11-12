#ifndef CHATCLIENTDIALOG_H
#define CHATCLIENTDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QListWidgetItem>

class PacketParser;
class PacketReceiver;

namespace Ui {
class ChatClientDialog;
}

class ChatClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatClientDialog(QWidget *parent = nullptr);
    ~ChatClientDialog();

private slots:
    void onReadyRead();

    void on_pushButton_Login_clicked();

    void on_pushButton_Connect_clicked();

    void on_pushButton_Disconnect_clicked();

    void on_pushButton_Send_clicked();

    void on_listWidget_Rooms_itemClicked(QListWidgetItem *item);

private:
    Ui::ChatClientDialog *ui;
    PacketParser* _parser;
    PacketReceiver* _receiver;
    QTcpSocket _socket;
};

#endif // CHATCLIENTDIALOG_H
