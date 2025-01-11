#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QDialog>
#include <QLabel>
#include "socketManager.h"
#include "cardManager.h"

namespace Ui {
class AdminWindow;
}

class AdminWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr, const QString &code = QString());
    ~AdminWindow();

private slots:
    void responseReceived(const QString &message);
    void on_endGameBtn_clicked();
    void socketDisconnected();
    void on_setTopicBtn_clicked();
    void on_startTalkBtn_clicked();
    void on_stopTalkBtn_clicked();

    void on_sendMessageBtn_clicked();

private:
    Ui::AdminWindow *ui;
    SocketManager *socketManager;
    CardManager *cardManager;
    QString code;
};

#endif // ADMINWINDOW_H
