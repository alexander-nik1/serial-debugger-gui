#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

private slots:

    void on_openPortButton_clicked();
    void on_closePortButton_clicked();
    void on_setBaudButton_clicked();
    void on_setDTRLowButton_clicked();
    void on_setDTRHighButton_clicked();
    void on_setRTSLowButton_clicked();
    void on_setRTSHighButton_clicked();
    void on_clearRxDButton_clicked();
    void on_sendTxDButton_clicked();

    void onSerialPortReadyRead();
    void onTimerTimeout();

    void on_pollingTimeEdit_returnPressed();

    void on_pollingTimeEdit_editingFinished();

private:
    Ui::MainWindow *ui;
    QSerialPort* serialPort;
    QTimer* pollingTimer;
    QLabel* statusLabel;
};

#endif // MAINWINDOW_H
