#ifndef CARDMANAGER_H
#define CARDMANAGER_H

#include "clickablelabel.h"
#include "socketManager.h"
#include <QObject>
#include <QLabel>
#include <QString>
#include <QMap>
#include <QList>

class CardManager : public QObject
{
    Q_OBJECT

public:

    static CardManager* getInstance();


    void addCard(const QString& playerName, QWidget* parent);
    void removeCard(const QString& playerName);
    void setNumberCard(const QString& playerName, int cardNumber);
    int getNumberCard(const QString& playerName) const;
    void showCard(const QString& playerName);
    void hideCard(const QString& playerName);
    void flipCard(const QString& playerName);
    void flipAllCards(bool faceUp);
    void generateChooseCards(QWidget* parent);
    void toggleChooseCards(bool enable);

signals:
    void cardClicked(int cardNumber);

private:

    explicit CardManager(QObject *parent = nullptr);
    ~CardManager();


    void updateCardPositions();


    static CardManager* instance;


    QMap<QString, QLabel*> playerCards;
    QMap<QString, QLabel*> playerNames;
    QMap<QString, int> cardNumbers;
    QMap<QString, bool> cardState;


    QList<QString> playerOrder;
    std::vector<ClickableLabel*> cards;

    const int cardHorizontalSize = 125;
    const int cardVerticalSize = 220;

    int chooseCardHorizontalSize = 100;
    int chooseCardVerticalSize = 180;
};

#endif
