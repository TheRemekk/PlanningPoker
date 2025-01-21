#include "adminwindow.h"
#include "ui_adminwindow.h"
#include "mainwindow.h"
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QTextEdit>

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

    ui->chatWidget->setStyleSheet(
        "QWidget#chatWidget {"
        "background-color: #874641;"
        "border: none;"
        "border-left: 3px solid black;"
        "}"
    );

    socketManager = SocketManager::getInstance();
    cardManager = CardManager::getInstance();
    connect(socketManager, &SocketManager::socketDisconnected, this, &AdminWindow::socketDisconnected);
    connect(socketManager, &SocketManager::responseReceived, this, &AdminWindow::responseReceived);

    ui->chatWidget->setVisible(false);
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
    QString textPlayerMessage = "message";

    QString sequence = message;
    sequence = sequence.replace("\n", " ");

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
        int numberCard = sequence.mid(index + wonGameMessage.length()).trimmed().toInt();
        std::vector<QString> cardPaintedSymbols = {"0", "1", "2", "3", "5", "8", "13", "21", "34", "55", "89", "?", "☕"};
        ui->resultLabel->setText("Wygrana: "+cardPaintedSymbols[numberCard-1]);
        ui->topicTextEdit->clear();
        ui->startTalkBtn->setEnabled(false);
        ui->setTopicBtn->setEnabled(true);
        ui->stopTalkBtn->setEnabled(false);
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
        cardManager->flipAllCards(false);
    }
}

void AdminWindow::socketDisconnected()
{
    QApplication::quit();
}

void AdminWindow::on_setTopicBtn_clicked()
{
    QString topicMessage = ui->setTopicTextEdit->toPlainText();
    QString command = "temat "+topicMessage;
    socketManager->writeCommand(command);
    ui->topicTextEdit->setText(topicMessage);
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
    ui->chatWidget->setVisible(false);
    ui->messageTextEdit->clear();
}

void AdminWindow::on_sendMessageBtn_clicked()
{
    QString playerMessage = ui->sendMessageLineEdit->text().replace("\n", " ").trimmed();
    QString command = "message "+playerMessage;
    socketManager->writeCommand(command);

    ui->messageTextEdit->setTextColor(Qt::blue);
    ui->messageTextEdit->append("Ty: "+playerMessage+"\n");
    ui->messageTextEdit->setTextColor(Qt::black);
    ui->sendMessageLineEdit->clear();
}

