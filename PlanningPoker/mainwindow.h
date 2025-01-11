#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include "socketManager.h"
#include "joingamewindow.h"
#include "adminwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QString pathToImages;

private slots:
    void on_joinGameBtn_clicked();
    void on_connectBtn_clicked();
    void socketConnected();
    void socketDisconnected();
    void socketError(const QString &errorMsg);
    void onReturnFromGame();
    void on_createGameBtn_clicked();

protected:
    QTimer * connTimeoutTimer{nullptr};

private:
    Ui::MainWindow *ui;
    JoinGameWindow *joinGameWindow = nullptr;
    AdminWindow *adminWindow = nullptr;
    SocketManager *socketManager;

};
#endif // MAINWINDOW_H
