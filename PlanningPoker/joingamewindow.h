#ifndef JOINGAMEWINDOW_H
#define JOINGAMEWINDOW_H

#include <QDialog>
#include "socketManager.h"
#include "playerwindow.h"

namespace Ui {
class JoinGameWindow;
}

class JoinGameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit JoinGameWindow(QWidget *parent = nullptr);
    ~JoinGameWindow();

private slots:
    void on_backBtn_clicked();
    void on_joinBtn_clicked();
    void socketDisconnected();
    void onNickLineEditChange(const QString &text);
    QStringList parseMessage(const QString& message);

signals:
    void returnToMainWindow();

private:
    Ui::JoinGameWindow *ui;
    SocketManager *socketManager;
    PlayerWindow *playerWindow = nullptr;
    QStringList playerNameList;
};

#endif
