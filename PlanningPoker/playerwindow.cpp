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
#include <QTextEdit>

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

    ui->chatWidget->setStyleSheet(
        "QWidget#chatWidget {"
        "background-color: #874641;"
        "border: none;"
        "border-left: 3px solid black;"
        "}"
    );

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

    ui->chatWidget->setVisible(false);
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
    QString textPlayerMessage = "message";

    QString sequence = message;
    sequence = sequence.replace("\n", " ");

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
        ui->chatWidget->setVisible(false);
        ui->messageTextEdit->clear();
    }
    if(sequence.contains(leavePlayerMessage)) {
        int index = sequence.indexOf(leavePlayerMessage);
        QString playerName = sequence.mid(index + leavePlayerMessage.length()).trimmed();
        cardManager->removeCard(playerName);
    }
    if(sequence.contains(selectedCardMessage)) {
        QString trimmedMessage = sequence.trimmed();
        QStringList parts = trimmedMessage.split(" ", Qt::SkipEmptyParts);
        QStringList cleanParts;

        for (const QString &part : parts) {
            if (part != "stop" && part != "selected" && part != "card" && part != "won") {
                cleanParts.append(part);
            }
        }

        for (int i = 0; i < cleanParts.size(); i += 2) {
            if (i + 1 < cleanParts.size()) {
                QString playerName = cleanParts[i];
                int cardNumber = cleanParts[i + 1].toInt();
                cardManager->setNumberCard(playerName, cardNumber);
                cardManager->showCard(playerName);
            }
        }
    }
    if(sequence.contains(wonGameMessage)) {
        int index = sequence.indexOf(wonGameMessage);
        int numberCard = sequence.mid(index + wonGameMessage.length()).trimmed().toInt();
        std::vector<QString> cardPaintedSymbols = {"0", "1", "2", "3", "5", "8", "13", "21", "34", "55", "89", "?", "☕"};
        ui->resultLabel->setText("Wygrana: "+cardPaintedSymbols[numberCard-1]);
        ui->topicTextEdit->clear();
        cardManager->toggleChooseCards(false);
    }
    if(sequence.contains(textPlayerMessage)) {
        int index = sequence.indexOf(textPlayerMessage);
        QString textMessage = sequence.mid(index + textPlayerMessage.length()).trimmed();
        QStringList parts = textMessage.split(" ");
        parts.removeAll("");

        if (parts.size() > 1) {
            QString playerName = parts.first();
            QString text = parts.mid(1).join(" ");
            ui->messageTextEdit->append(playerName + ": " + text);
        }
    }
    if(sequence.contains(overtimeGameMessage)) {
        ui->resultLabel->setText("Dogrywka");
        ui->chatWidget->setVisible(true);
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

void PlayerWindow::on_sendMessageBtn_clicked()
{
    QString playerMessage = ui->sendMessageLineEdit->text().replace("\n", " ").trimmed();
    QString command = "message "+playerMessage;
    socketManager->writeCommand(command);

    ui->messageTextEdit->setTextColor(Qt::blue);
    ui->messageTextEdit->append("Ty: "+playerMessage+"\n");
    ui->messageTextEdit->setTextColor(Qt::black);
    ui->sendMessageLineEdit->clear();
}

