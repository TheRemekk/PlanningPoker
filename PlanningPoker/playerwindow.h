#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QStringList>
#include "socketManager.h"
#include "cardManager.h"

namespace Ui {
class PlayerWindow;
}

class PlayerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PlayerWindow(QWidget *parent = nullptr, const QString &code = QString(), const QStringList& playerNameList = QStringList());
    ~PlayerWindow();

private slots:
    void responseReceived(const QString &message);
    void socketDisconnected();
    void on_leaveGameBtn_clicked();

public slots:
    void onCardClicked(int cardNumber);

private:
    Ui::PlayerWindow *ui;
    SocketManager *socketManager;
    CardManager *cardManager;
    QString code;
    QString playerName;
    QStringList playerNameList;
    QString topic;

    bool isStarted = false;
};

#endif // PLAYERWINDOW_H
