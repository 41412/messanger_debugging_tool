#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _client1(1)
{
    ui->setupUi(this);
    ui->textEdit_RxHex->setFontFamily("Courier New");
    ui->textEdit_RxText->setFontFamily("Courier New");
    //ui->textEdit_RxHex->setText("00 11 22 33 44 55 66 77\naa bb cc dd ee ff gg hh");

    connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    connect(&_client1, SIGNAL(onRead(const QByteArray&)), this, SLOT(onClient1Recieved(const QByteArray&)));
}

MainWindow::~MainWindow()
{
    closeSessions();
    delete ui;
}

void MainWindow::onNewConnection()
{
    QTcpSocket *clientSocket = _server.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

    _sockets.push_back(clientSocket);
    // for (QTcpSocket* socket : _sockets) {
    //    socket->write(QByteArray::fromStdString(clientSocket->peerAddress().toString().toStdString() + " connected to server !\n"));
    //}

    qDebug("A new connection!!!");
    qDebug() << "total " << _sockets.count() << "connected";
}

void MainWindow::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        _sockets.removeOne(sender);
        qDebug("A connection has been released");
        qDebug() << "total " << _sockets.count() << "connected";
    }
}

void MainWindow::onReadyRead()
{
    ui->textEdit_RxHex->setText("");
    ui->textEdit_RxText->setText("");

    for (int i = 0; i < _sockets.count(); i++) {
        QTcpSocket* socket = _sockets[i];
        QByteArray data = socket->readAll();

        QString str, str2;
        QString tm;

        for (int i = 0; i < data.count(); i++) {
            if (i%8 == 0 && i != 0) {
                str += "\r\n      ";
                str2 += "\r\n      ";
            }
            str += QString::asprintf("%02X ", static_cast<unsigned char>(data[i]));
            str2 += QString::asprintf("%c ", static_cast<unsigned char>(data[i]));
        }
        str.prepend(QString::asprintf("[%04d]",i));
        str2.prepend(QString::asprintf("[%04d]",i));
        ui->textEdit_RxHex->setText(str);
        ui->textEdit_RxText->setText(str2);
    }
}

void MainWindow::onClient1Recieved(const QByteArray& data)
{
    ui->textEdit_RxHex->setText("");
    ui->textEdit_RxText->setText("");

    QString str, str2;
    QString tm;

    str.clear();
    for (int i = 0; i < data.count(); i++) {
        if (i%8 == 0 && i != 0) {
            str += "\r\n";
            str2 += "\r\n";
        }
        str += QString::asprintf("%02X ", static_cast<unsigned char>(data[i]));
        str2 += QString::asprintf("%c ", static_cast<unsigned char>(data[i]));
    }
    ui->textEdit_RxHex->setText(str);
    ui->textEdit_RxText->setText(str2);
}

void MainWindow::on_pushButton_AsServer_clicked()
{
    openSession(true);
}

void MainWindow::on_pushButton_AsClient_clicked()
{
    openSession(false);
}

void MainWindow::on_pushButton_Stop_clicked()
{
    closeSessions();
}

static inline QByteArray IntToArray(int v) {
    QByteArray bA;
    QDataStream stream(&bA, QIODevice::WriteOnly);
    stream << v;
    return bA;
}

void MainWindow::on_pushButton_SendData_clicked()
{
    QByteArray ba;
    QString str = ui->textEdit_RawData->toPlainText().toLower();
    str.replace(" ", "");
    str.replace("\r", "");
    str.replace("\n", "");
    ba = QByteArray::fromHex(str.toUtf8());

    if (_client1.isOpen()) {
        _client1.send(ba);
    }
    else {
        for (int i = 0; i < _sockets.count(); i++) {
            if (_sockets[i]->isWritable() && _sockets[i]->isOpen()) {
                _sockets[i]->write(ba);
            }
        }
    }
}

void MainWindow::openSession(bool asServer)
{
    unsigned int port = ui->lineEdit_Port->text().toUInt();
    if (asServer) {
        if (!_server.isListening()) {
            _server.listen(QHostAddress::Any, port);
            qDebug() << "Started server!!!\n";
        }
    }
    else {
        QString addr = ui->lineEdit_Address->text();
        if (_client1.connectTo(addr, port)) {
            qDebug("Started client");
        }
    }
}

void MainWindow::closeSessions()
{
    if (_client1.isOpen()) {
        _client1.disconnectFrom();
        _client1.close();
        qDebug("closed client session\n");
    }
    if (_server.isListening()) {
        _server.close();
        _sockets.clear();
    }
}

void MainWindow::updateRawData()
{
    QByteArray d;
    QString t;
    QString id = ui->lineEdit_Pmid->text();
    QString msg = ui->textEdit_Items->toPlainText();

    // space
    if (!id.isEmpty()) {
        d += ' ';

        // protocol : int
        t = id;
        d += t.toUtf8();
    }

    if (!msg.isEmpty()) {
        // items : separated by space
        QStringList items = msg.split(' ');
        for (auto it: items) {
            d += ' ';

            if ((it.length() > 2) && (it[0] == '`' && it[1] == '#')) {
                QString rem = it.right(it.length() - 2);
                d += IntToArray(rem.toInt());
            }
            else {
                d += it.toUtf8();
            }
        }
    }

    if (d.size() != 0) {
        // set size
        d.prepend(IntToArray(d.count()));

        t = "";
        for(auto e: d) {
            t += QString::asprintf("%02X ", static_cast<unsigned char>(e));
        }
    }

    ui->textEdit_RawData->setText(t);
}

void MainWindow::on_lineEdit_Pmid_textChanged(const QString&)
{
    updateRawData();
}

void MainWindow::on_textEdit_Items_textChanged()
{
    updateRawData();
}

void MainWindow::on_pushButton_sigLoginSuccess_clicked()
{
    ui->lineEdit_Pmid->setText("LOGIN_SUCCESS");
}

void MainWindow::on_pushButton_sig_UserdataSendStart_clicked()
{
    ui->lineEdit_Pmid->setText("USERDATA_SEND_START");
}

void MainWindow::on_pushButton_sig_SendFriendlist_clicked()
{
    ui->lineEdit_Pmid->setText("SEND_FRIENDLIST");
}

void MainWindow::on_pushButton_sig_UserdataSendEnd_clicked()
{
    ui->lineEdit_Pmid->setText("USERDATA_SEND_END");
}


