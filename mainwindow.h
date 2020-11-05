#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "hmclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onNewConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState state);
    void onReadyRead();

private slots:
    void onClient1Recieved(const QByteArray&);
    void on_pushButton_AsServer_clicked();
    void on_pushButton_AsClient_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_SendData_clicked();
    void on_lineEdit_Pmid_textChanged(const QString &arg1);
    void on_textEdit_Items_textChanged();

    void on_pushButton_sigLoginSuccess_clicked();
    void on_pushButton_sig_UserdataSendStart_clicked();
    void on_pushButton_sig_SendFriendlist_clicked();
    void on_pushButton_sig_UserdataSendEnd_clicked();

    void on_tableWidget_cellClicked(int row, int column);

    void on_pushButton_SendFriendListHuge_clicked();

private:
    Ui::MainWindow *ui;

private:
    struct TestItems{
        QString title;
        QString dir;
        QString id;
        QString msg;
    };

    QTcpServer _server;
    QList<QTcpSocket*> _sockets;
    HmClient _client1;
    QVector<TestItems> _presets;

    void openSession(bool asServer);
    void closeSessions();
    void updateRawData();
    bool loadPresets();
};
#endif // MAINWINDOW_H
