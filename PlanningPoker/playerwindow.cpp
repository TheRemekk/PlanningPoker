#include "playerwindow.h"
#include "ui_playerwindow.h"
#include "mainwindow.h"
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QString>
#include <QStringList>

PlayerWindow::PlayerWindow(QWidget *parent, const QString &code, const QStringList &playerNameList)
    : QDialog(parent)
    , ui(new Ui::PlayerWindow)
    , code(code)
    , playerNameList(playerNameList)
{
    ui->setupUi(this);
    this->setFixedSize(1600, 900);
    QPixmap pix(MainWindow::pathToImages+"/3.png");
    ui->planningTitle->setPixmap(pix);
    ui->codeLineEdit->setText(code);

    setStyleSheet("QGroupBox {"
        "background-color: #874641;"
        "border: none;"
        "border-top: 3px solid black;"
        "border-bottom: 3px solid black;"
    "}"
    "PlayerWindow{"
        "background-color: #ad6b66"
    "}");

    socketManager = SocketManager::getInstance();
    cardManager = CardManager::getInstance();
    connect(socketManager, &SocketManager::socketDisconnected, this, &PlayerWindow::socketDisconnected);
    connect(socketManager, &SocketManager::responseReceived, this, &PlayerWindow::responseReceived);
    connect(cardManager, &CardManager::cardClicked, this, &PlayerWindow::onCardClicked);

    playerName = playerNameList.first();
    ui->playerNameLabel->setText(playerName);

    for (const QString& player : playerNameList) {
        cardManager->addCard(player, this);
    }

    cardManager->generateChooseCards(this);
    cardManager->toggleChooseCards(false);
}

PlayerWindow::~PlayerWindow()
{
    delete ui;
}

void PlayerWindow::on_leaveGameBtn_clicked()
{
    QString command = "leave";
    socketManager->writeCommand(command);
    QApplication::quit();
}


void PlayerWindow::responseReceived(const QString &message)
{
    QString joinPlayerMessage = "joined";
    QString leavePlayerMessage = "leave";
    QString startMessage = "start";
    QString stopMessage = "stop";
    QString selectedCardMessage = "selected card";
    QString wonGameMessage = "won";
    QString overtimeGameMessage = "overtime";
    QString topicMessage = "temat";

    QString sequence = message;

    if(sequence.contains(joinPlayerMessage)) {
        int index = sequence.indexOf(joinPlayerMessage);
        QString playerName = sequence.mid(index + joinPlayerMessage.length()).trimmed();
        cardManager->addCard(playerName, this);
    }
    if(sequence.contains(topicMessage)) {
        int index = sequence.indexOf(topicMessage);
        QString topic = sequence.mid(index + topicMessage.length()).trimmed();
        ui->topicTextEdit->setText(topic);
        ui->resultLabel->setText("");
    }
    if(sequence.contains(startMessage)) {
        cardManager->toggleChooseCards(true);
        cardManager->flipAllCards(false);
    }
    if(sequence.contains(stopMessage)) {
        cardManager->toggleChooseCards(false);
        ui->topicTextEdit->clear();

        int startIndex = sequence.indexOf(selectedCardMessage) + selectedCardMessage.length();

        QString neededData = sequence.mid(startIndex).trimmed();

        int endIndex = neededData.indexOf("\n");
        if (endIndex != -1) {
            neededData = neededData.left(endIndex).trimmed();
        }

        QStringList parts = neededData.split(" ");
        if (parts.size() % 2 != 0) {
            qDebug() << "Nieprawidłowy format sekwencji!";
            return;
        }

        for (int i = 0; i < parts.size(); i += 2) {
            QString playerName = parts[i];
            int cardNumber = parts[i + 1].toInt();

            cardManager->setNumberCard(playerName, cardNumber);
            cardManager->showCard(playerName);
        }
    }
    if(sequence.contains(leavePlayerMessage)) {
        int index = sequence.indexOf(leavePlayerMessage);
        QString playerName = sequence.mid(index + leavePlayerMessage.length()).trimmed();
        cardManager->removeCard(playerName);
    }
    if(sequence.contains(wonGameMessage)) {
        int index = sequence.indexOf(wonGameMessage);
        QString numberCard = sequence.mid(index + wonGameMessage.length()).trimmed();
        ui->resultLabel->setText("Wygrana: "+numberCard);
    }
    if(sequence.contains(overtimeGameMessage)) {
        ui->resultLabel->setText("Dogrywka");
    }
}

void PlayerWindow::socketDisconnected()
{
    QApplication::quit();
}

void PlayerWindow::onCardClicked(int cardNumber)
{
    QString command = "selected card "+ QString::number(cardNumber);
    socketManager->writeCommand(command);
}
