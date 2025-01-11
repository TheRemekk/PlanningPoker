#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QPixmap>

QString MainWindow::pathToImages = "/root/Dokumenty/QtCreator/PlanningPoker/images/";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(480, 600);

    setStyleSheet("MainWindow {"
        "background-color: #ad6b66"
    "}");

    QPixmap planningLogoPix(pathToImages+"4.png");
    ui->planningLogo->setPixmap(planningLogoPix);
    QPixmap planningTitlePix(pathToImages+"3.png");
    ui->planningTitle->setPixmap(planningTitlePix);

    socketManager = SocketManager::getInstance();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectBtn_clicked()
{
    ui->connectGroup->setEnabled(false);
    connTimeoutTimer = new QTimer(this);
    connTimeoutTimer->setSingleShot(true);

    connect(connTimeoutTimer, &QTimer::timeout, [&] {
        socketManager->disconnectFromServer();
        connTimeoutTimer->deleteLater();
        connTimeoutTimer = nullptr;
        ui->connectGroup->setEnabled(true);
        QMessageBox::critical(this, "Błąd", "Nie udało się połączyć z serwerem: Connection timed out");
    });

    connect(socketManager, &SocketManager::socketConnected, this, &MainWindow::socketConnected);
    connect(socketManager, &SocketManager::socketDisconnected, this, &MainWindow::socketDisconnected);


    QString host = ui->hostLineEdit->text();
    quint16 port = ui->portSpinBox->value();
    socketManager->connectToServer(host, port);

    connTimeoutTimer->start(3000);
}

void MainWindow::on_createGameBtn_clicked()
{
    QString command = "new game";
    QString response = socketManager->writeCommandAndReceiveResponse(command);
    QString code = response.remove(0, (response.length()-4)-1);


    if (adminWindow) {
        adminWindow->deleteLater();
    }

    adminWindow = new AdminWindow(this, code);
    adminWindow->setWindowFlags(Qt::Window);
    adminWindow->show();
    this->hide();
}

void MainWindow::on_joinGameBtn_clicked()
{
    if (joinGameWindow) {
        joinGameWindow->deleteLater();
    }

    joinGameWindow = new JoinGameWindow(this);
    connect(joinGameWindow, &JoinGameWindow::returnToMainWindow, this, &MainWindow::onReturnFromGame);

    joinGameWindow->setWindowFlags(Qt::Window);
    joinGameWindow->show();
    this->hide();
}

void MainWindow::socketConnected(){
    connTimeoutTimer->stop();
    connTimeoutTimer->deleteLater();
    connTimeoutTimer=nullptr;
    ui->chooseGameGroup->setEnabled(true);
}

void MainWindow::socketDisconnected(){
    ui->chooseGameGroup->setEnabled(false);
    ui->connectGroup->setEnabled(true);
}

void MainWindow::socketError(const QString &errorMsg) {
    QMessageBox::critical(this, "Błąd połączenia", errorMsg);
    ui->chooseGameGroup->setEnabled(false);
    ui->connectGroup->setEnabled(true);
}

void MainWindow::onReturnFromGame() {
    show();
}


