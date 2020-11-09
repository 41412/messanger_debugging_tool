#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QCryptographicHash>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _client1(1)
{
    ui->setupUi(this);
    ui->textEdit_RxHex->setFontFamily("Courier New");
    ui->textEdit_RxText->setFontFamily("Courier New");

    ui->tableWidget->setColumnCount(3);
    QStringList sl = { "Title", "ID", "Msg" };
    ui->tableWidget->setHorizontalHeaderLabels(sl);
    //ui->textEdit_RxHex->setText("00 11 22 33 44 55 66 77\naa bb cc dd ee ff gg hh");

    connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    connect(&_client1, SIGNAL(onRead(const QByteArray&)), this, SLOT(onClient1Recieved(const QByteArray&)));

    loadPresets("test.json");
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
        d.prepend("MR2020");
        d.prepend(0xEE);
        d.prepend(0xFF);
        //d.prepend(QString::asprintf("%d ", d.count()).toUtf8());

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


bool MainWindow::loadPresets(const QString& filepath)
{
    QFile loadFile(filepath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc = QJsonDocument::fromJson(saveData);

    QJsonObject json = loadDoc.object();
    QJsonArray arr = json["scenes"].toArray();
    QJsonArray::iterator it = arr.begin();
    //QList<TestItems> list;

    if (arr.count() == 0) {
        QMessageBox msgBox;
        msgBox.setText("Invalid sceniro file");
        msgBox.exec();
        return false;
    }

    ui->tableWidget->setRowCount(0);
    for (; it != arr.end(); it++) {
        TestItems item;
        QJsonObject obj = it->toObject();
        item.title = obj["title"].toString();
        item.dir = obj["dir"].toString();
        item.id = obj["id"].toString();
        item.msg = obj["msg"].toString();

        //_presets.push_back(item);

        int pos = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(pos);
        QTableWidgetItem *twi1 =new QTableWidgetItem(item.title);
        QTableWidgetItem *twi2 =new QTableWidgetItem(item.id);
        QTableWidgetItem *twi3 =new QTableWidgetItem(item.msg);
        ui->tableWidget->setItem(pos, 0, twi1);
        ui->tableWidget->setItem(pos, 1, twi2);
        ui->tableWidget->setItem(pos, 2, twi3);
    }
    return true;
}


void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    // qDebug() << row << ", " << column;
    ui->lineEdit_Pmid->setText(ui->tableWidget->item(row, 1)->text());
    ui->textEdit_Items->setText(ui->tableWidget->item(row, 2)->text());
}

void MainWindow::on_pushButton_SendFriendListHuge_clicked()
{
    QString str;
    int count = 0xffff;
    for (int i = 0; i < count; i++) {
        str += QString::asprintf(" TestUser%d", i);
    }
    ui->lineEdit_Pmid->setText("SEND_FRIENDLIST");
    str.prepend(QString::asprintf("%d", count));
    ui->textEdit_Items->setText(str);
}

void MainWindow::on_pushButton_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this,tr("Open Scenario file"), "", tr("Image Files (*.json)"));
    //qDebug() << filepath;
    loadPresets(filepath);
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    QTableWidget* tw = ui->tableWidget;
    for (int r = 0; r < ui->tableWidget->rowCount(); r++) {
        if (tw->item(r, 0)->text().contains(arg1,Qt::CaseSensitivity::CaseInsensitive) ||
            tw->item(r, 1)->text().contains(arg1,Qt::CaseSensitivity::CaseInsensitive) ||
            tw->item(r, 2)->text().contains(arg1,Qt::CaseSensitivity::CaseInsensitive) ||
            arg1.isEmpty())
        {
            ui->tableWidget->setRowHidden(r, false);
        }
        else
        {
            ui->tableWidget->setRowHidden(r, true);
        }
    }
}
