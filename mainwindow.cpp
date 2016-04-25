#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <unistd.h>

enum { DEFAULT_POLLING_TIME = 250 };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialPort(NULL)
{
    ui->setupUi(this);

    pollingTimer = new QTimer(this);
    pollingTimer->setSingleShot(false);
    pollingTimer->setTimerType(Qt::PreciseTimer);

    pollingTimer->setInterval(DEFAULT_POLLING_TIME);

    QObject::connect(pollingTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));

    pollingTimer->start();

    ui->pollingTimeEdit->setText(QString::number(DEFAULT_POLLING_TIME));

    statusLabel = new QLabel("Serial port closed", this);
    ui->statusbar->addWidget(statusLabel, 0);
}

MainWindow::~MainWindow()
{
    if(pollingTimer)
        delete pollingTimer;

    if(statusLabel)
        delete statusLabel;

    on_closePortButton_clicked();

    delete ui;
}

void MainWindow::on_openPortButton_clicked()
{
    if(serialPort != NULL)
    {
        qWarning("%s: Serial port already opened, I will close it", __PRETTY_FUNCTION__);
        statusLabel->setText("Reopening serial port ...");
        on_closePortButton_clicked();
    }

    serialPort = new QSerialPort(this);


    serialPort->setPortName(ui->deviceEdit->text());
    serialPort->setReadBufferSize(128);

    if(!serialPort->open(QIODevice::ReadWrite)) {
        QString err;
        err.sprintf("Error with open serial port \"%s\": %s",
                    qPrintable(ui->deviceEdit->text()),
                    qPrintable(serialPort->errorString()));
        qCritical() << err;
        on_closePortButton_clicked();
        statusLabel->setText(err);
        return;
    }

    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setTextModeEnabled(false);

    QObject::connect(serialPort, SIGNAL(readyRead()), this, SLOT(onSerialPortReadyRead()));

    statusLabel->setText("Serial port opened");

    ui->openPortButton->setEnabled(false);
    ui->closePortButton->setEnabled(true);
}

void MainWindow::on_closePortButton_clicked()
{
    if(serialPort)
    {
        if(serialPort->isOpen())
            serialPort->close();
        delete serialPort;
        serialPort = NULL;
        statusLabel->setText("Serial port closed");

        ui->openPortButton->setEnabled(true);
        ui->closePortButton->setEnabled(false);
    }
    else
        qWarning("%s: Serial port was not opened!", __PRETTY_FUNCTION__);
}

void MainWindow::on_setBaudButton_clicked()
{
    if(!serialPort)
        return;

    bool ok;
    const int baud = ui->baudrateEdit->text().toInt(&ok);

    if(ok && baud > 0)
    {
        serialPort->setBaudRate(baud);
        QString s;
        s.sprintf("Baudrate changed to %d", baud);
        statusLabel->setText(s);
    }
    else
    {
        QString s;
        s.sprintf("\"%s\" is NOT valid a baudrate value", qPrintable(ui->baudrateEdit->text()));
        qCritical() << s;
        statusLabel->setText(s);
    }
}

void MainWindow::on_setDTRLowButton_clicked()
{
    if(!serialPort)
        return;

    serialPort->setDataTerminalReady(false);
}

void MainWindow::on_setDTRHighButton_clicked()
{
    if(!serialPort)
        return;

    serialPort->setDataTerminalReady(true);
}

void MainWindow::on_setRTSLowButton_clicked()
{
    if(!serialPort)
        return;

    serialPort->setRequestToSend(false);
}

void MainWindow::on_setRTSHighButton_clicked()
{
    if(!serialPort)
        return;

    serialPort->setRequestToSend(true);
}

void MainWindow::on_clearRxDButton_clicked()
{
    ui->receivedDataEdit->clear();
}

void MainWindow::on_sendTxDButton_clicked()
{
    if(!serialPort)
        return;

    qint64 bytesSent;

    bytesSent = serialPort->write(ui->dataToSendEdit->toPlainText().toUtf8());

    QString s;
    s.sprintf("Sent %lld bytes", bytesSent);
    statusLabel->setText(s);

    if(ui->clearAfterSendCheck->isChecked())
        ui->dataToSendEdit->clear();
}

void MainWindow::onSerialPortReadyRead()
{
    if(!serialPort)
        return;

    if(ui->clearBeforeReceiveCheck->isChecked())
        ui->receivedDataEdit->clear();

    const QByteArray ba = serialPort->readAll();
    ui->receivedDataEdit->moveCursor(QTextCursor::End);
    ui->receivedDataEdit->insertPlainText(QString(ba));
    ui->receivedDataEdit->moveCursor(QTextCursor::End);

    QString s;
    s.sprintf("Received %d bytes", ba.length());
    statusLabel->setText(s);
}

void setEditSignalValue(QLineEdit* _le, int _code)
{
    switch(_code)
    {
    case 0:
        _le->setStyleSheet("color: yellow; background-color: black");
        _le->setText("0");
        break;

    case 1:
        _le->setStyleSheet("color: black; background-color: yellow");
        _le->setText("1");
        break;

    default:
        _le->setStyleSheet("color: black; background-color: black");
        _le->setText("");
        break;
    }
}

#define CHECK_FLAG(_flag_)  \
    (((ps & QSerialPort::_flag_) != 0) ? 1 : 0)

void MainWindow::onTimerTimeout()
{
    if(serialPort)
    {
        const QSerialPort::PinoutSignals ps = serialPort->pinoutSignals();
        setEditSignalValue(ui->DTRShowEdit, CHECK_FLAG(DataTerminalReadySignal));
        setEditSignalValue(ui->DCDShowEdit, CHECK_FLAG(DataCarrierDetectSignal));
        setEditSignalValue(ui->DSRShowEdit, CHECK_FLAG(DataSetReadySignal));
        setEditSignalValue(ui->RIShowEdit, CHECK_FLAG(RingIndicatorSignal));
        setEditSignalValue(ui->RTSShowEdit, CHECK_FLAG(RequestToSendSignal));
        setEditSignalValue(ui->CTSShowEdit, CHECK_FLAG(ClearToSendSignal));
    }
    else
    {
        setEditSignalValue(ui->DTRShowEdit, -1);
        setEditSignalValue(ui->DCDShowEdit, -1);
        setEditSignalValue(ui->DSRShowEdit, -1);
        setEditSignalValue(ui->RIShowEdit, -1);
        setEditSignalValue(ui->RTSShowEdit, -1);
        setEditSignalValue(ui->CTSShowEdit, -1);
    }
}

void MainWindow::on_pollingTimeEdit_returnPressed()
{
    bool ok;
    const unsigned int t = ui->pollingTimeEdit->text().toUInt(&ok);

    if(ok && t)
    {
        pollingTimer->setInterval(t);
        QString s;
        s.sprintf("Polling time changed to %d milliseconds", t);
        statusLabel->setText(s);
    }
    else
    {
        QString s;
        s.sprintf("\"%s\" is NOT valid a polling time value", qPrintable(ui->pollingTimeEdit->text()));
        qCritical() << s;
        statusLabel->setText(s);
    }
}

void MainWindow::on_pollingTimeEdit_editingFinished()
{
    on_pollingTimeEdit_returnPressed();
}
