#include "adminwindow.h"
#include "ui_adminwindow.h"
#include "mainwindow.h"
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>

AdminWindow::AdminWindow(QWidget *parent, const QString &code)
    : QDialog(parent)
    , ui(new Ui::AdminWindow)
    , code(code)
{
    ui->setupUi(this);
    this->setFixedSize(1600, 900);
    QPixmap pix(MainWindow::pathToImages+"/3.png");
    ui->planningTitle->setPixmap(pix);

    ui->codeLineEdit->setText(code);
    ui->startTalkBtn->setEnabled(false);
    ui->stopTalkBtn->setEnabled(false);

    setStyleSheet("QGroupBox {"
        "background-color: #874641;"
        "border: none;"
        "border-top: 3px solid black;"
        "border-bottom: 3px solid black;"
    "}"
    "AdminWindow{"
        "background-color: #ad6b66"
    "}");

    socketManager = SocketManager::getInstance();
    cardManager = CardManager::getInstance();
    connect(socketManager, &SocketManager::socketDisconnected, this, &AdminWindow::socketDisconnected);
    connect(socketManager, &SocketManager::responseReceived, this, &AdminWindow::responseReceived);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::on_endGameBtn_clicked()
{
    QString command = "end game";
    socketManager->writeCommand(command);
    QApplication::quit();
}


void AdminWindow::responseReceived(const QString &message)
{
    QString joinPlayerMessage = "joined";
    QString leavePlayerMessage = "leave";
    QString playerSelectedCardMessage = "selected card";
    QString wonGameMessage = "won";
    QString overtimeGameMessage = "overtime";

    QString sequence = message;

    if(sequence.contains(joinPlayerMessage)) {
        int index = sequence.indexOf(joinPlayerMessage);
        QString playerName = sequence.mid(index + joinPlayerMessage.length()).trimmed();
        cardManager->addCard(playerName, this);
    }
    if(sequence.contains(playerSelectedCardMessage) && !sequence.contains(overtimeGameMessage) && !sequence.contains(wonGameMessage)) {
        int index = sequence.indexOf(playerSelectedCardMessage);
        QString playerInfo = sequence.mid(index + playerSelectedCardMessage.length()).trimmed();
        QString separator = " ";
        QStringList parts = playerInfo.split(separator);

        QString playerName = parts.at(0);
        int cardNumber = parts.at(1).toInt();

        cardManager->setNumberCard(playerName, cardNumber);
        cardManager->showCard(playerName);
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

void AdminWindow::socketDisconnected()
{
    QApplication::quit();
}

void AdminWindow::on_setTopicBtn_clicked()
{
    topic = ui->setTopicTextEdit->toPlainText();
    QString command = "temat "+topic;
    socketManager->writeCommand(command);
    ui->topicTextEdit->setText(topic);
    ui->startTalkBtn->setEnabled(true);
    ui->resultLabel->setText("");
}


void AdminWindow::on_startTalkBtn_clicked()
{
    QString command = "start";
    socketManager->writeCommand(command);
    ui->startTalkBtn->setEnabled(false);
    ui->setTopicBtn->setEnabled(false);
    ui->stopTalkBtn->setEnabled(true);
    cardManager->flipAllCards(false);
}


void AdminWindow::on_stopTalkBtn_clicked()
{
    QString command = "stop";
    socketManager->writeCommand(command);
    topic = "";
    ui->topicTextEdit->clear();
    ui->startTalkBtn->setEnabled(false);
    ui->setTopicBtn->setEnabled(true);
    ui->stopTalkBtn->setEnabled(false);
}
