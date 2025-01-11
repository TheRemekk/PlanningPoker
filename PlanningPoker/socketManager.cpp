#include "socketManager.h"
#include <QElapsedTimer>
#include <QCoreApplication>

SocketManager* SocketManager::instance = nullptr;

SocketManager* SocketManager::getInstance()
{
    if (!instance) {
        instance = new SocketManager();
    }
    return instance;
}

SocketManager::SocketManager(QObject *parent) : QObject(parent), sock(new QTcpSocket(this))
{
    connect(sock, &QTcpSocket::connected, this, &SocketManager::onConnected);
    connect(sock, &QTcpSocket::disconnected, this, &SocketManager::onDisconnected);
    connect(sock, &QTcpSocket::errorOccurred, this, &SocketManager::onErrorOccurred);
    connect(sock, &QTcpSocket::readyRead, this, &SocketManager::onReadyRead);
}

SocketManager::~SocketManager()
{
    if (sock) {
        sock->deleteLater();
    }
}

QTcpSocket* SocketManager::getSocket()
{
    return sock;
}

void SocketManager::connectToServer(const QString &host, quint16 port)
{
    if (sock->state() == QTcpSocket::ConnectedState) {
        return;
    }

    sock->connectToHost(host, port);
}

void SocketManager::disconnectFromServer()
{
    if (sock->state() == QTcpSocket::ConnectedState) {
        sock->disconnectFromHost();
    }
}

void SocketManager::writeCommand(const QString &command)
{
    if (sock->state() == QTcpSocket::ConnectedState) {
        sock->write(command.toUtf8());
    }
}

QString SocketManager::writeCommandAndReceiveResponse(const QString &command)
{
    sock->readAll();
    m_response.clear();

    writeCommand(command);
    while (m_response.isEmpty()) {

        QCoreApplication::processEvents();
    }

    return m_response;
}


void SocketManager::onConnected()
{
    emit socketConnected();
}

void SocketManager::onDisconnected()
{
    sock->abort();  // Zatrzymujemy wszystkie operacje socketu
    sock->deleteLater();  // Usuwamy socket w bezpieczny sposób
    sock = nullptr;  // Ustawiamy wskaźnik na nullptr
    emit socketDisconnected();
}

void SocketManager::onErrorOccurred(QAbstractSocket::SocketError errorCode)
{
    QString errorMsg = sock->errorString();
    qDebug() << "Kod błędu:" << errorCode;
    qDebug() << "Opis błędu:" << errorMsg;

    emit socketError("Błąd połączenia: " + errorMsg);
}

void SocketManager::onReadyRead()
{
    QByteArray data = sock->readAll();
    m_response = QString(data);
    emit responseReceived(m_response);
}
