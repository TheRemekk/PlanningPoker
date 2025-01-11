#include "joingamewindow.h"
#include "ui_joingamewindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QApplication>

JoinGameWindow::JoinGameWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JoinGameWindow)
{
    ui->setupUi(this);
    this->setFixedSize(480, 600);
    QPixmap pix(MainWindow::pathToImages+"/4.png");
    ui->planningLogo->setPixmap(pix);

    setStyleSheet("JoinGameWindow{"
        "background-color: #ad6b66"
    "}");

    socketManager = SocketManager::getInstance();
    connect(socketManager, &SocketManager::socketDisconnected, this, &JoinGameWindow::socketDisconnected);
    connect(ui->nickLineEdit, &QLineEdit::textEdited, this, &JoinGameWindow::onNickLineEditChange);
}

JoinGameWindow::~JoinGameWindow()
{
    delete ui;
}

void JoinGameWindow::on_backBtn_clicked()
{
    emit returnToMainWindow();
    hide();
}


void JoinGameWindow::on_joinBtn_clicked()
{
    QString code = ui->codeLineEdit->text();
    QString playerName = ui->nickLineEdit->text();
    QString command = "join " + code + " " + playerName;
    QString joinPlayerMessage = "players";
    QString response = socketManager->writeCommandAndReceiveResponse(command);
    QString trimmedResponse = response.trimmed();

    if (trimmedResponse == "error game not found") {
        QMessageBox::information(this, "Error", "Nie znaleziono gry o podanym przez ciebie kodzie! Spróbuj ponownie.");
    }
    else if (trimmedResponse == "error player nick exists") {
        QMessageBox::information(this, "Error", "W tej grze istnieje już gracz o pseudonimie " + playerName + "! Spróbuj ponownie.");
    }
    else if (trimmedResponse.contains(playerName)) {
        playerNameList = parseMessage(trimmedResponse);
        if (playerWindow) {
            playerWindow->deleteLater();
        }
        playerWindow = new PlayerWindow(this, code, playerNameList);
        playerWindow->setWindowFlags(Qt::Window);
        playerWindow->show();
        this->hide();
    }
}

void JoinGameWindow::onNickLineEditChange(const QString &text)
{
    QString withoutSpaces = text;
    withoutSpaces.replace(" ", "");
    ui->nickLineEdit->setText(withoutSpaces);
}

QStringList JoinGameWindow::parseMessage(const QString& message) {
    QStringList parts = message.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty() || parts.first() != "players") {
        return {};
    }

    parts.removeFirst();
    parts.removeAll("admin");
    return parts;
}

void JoinGameWindow::socketDisconnected()
{
    QApplication::quit();
}

