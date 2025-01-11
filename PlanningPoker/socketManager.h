#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QObject>
#include <QTcpSocket>

class SocketManager : public QObject
{
    Q_OBJECT

public:
    static SocketManager* getInstance();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void writeCommand(const QString &command);
    QString writeCommandAndReceiveResponse(const QString &command);
    QTcpSocket* getSocket();

signals:
    void socketConnected();
    void socketDisconnected();
    void socketError(const QString &errorString);
    void responseReceived(const QString &data);

private:
    explicit SocketManager(QObject *parent = nullptr);
    ~SocketManager();

    static SocketManager *instance;

    QTcpSocket *sock;
    QString m_response;

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReadyRead();
};

#endif // SOCKETMANAGER_H
