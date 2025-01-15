#include "cardManager.h"
#include "mainwindow.h"
#include "clickablelabel.h"
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <vector>

CardManager* CardManager::instance = nullptr;

CardManager* CardManager::getInstance()
{
    if (!instance) {
        instance = new CardManager();
    }
    return instance;
}

CardManager::CardManager(QObject *parent) : QObject(parent)
{
}

CardManager::~CardManager()
{
    qDeleteAll(playerCards);
    qDeleteAll(playerNames);
    playerCards.clear();
    playerNames.clear();
    cardNumbers.clear();
    cardState.clear();
    playerOrder.clear();
}

void CardManager::addCard(const QString& playerName, QWidget* parent)
{
    if (playerCards.contains(playerName)) {
        return;
    }

    QPixmap card(MainWindow::pathToImages+"1.png");
    card = card.scaled(cardHorizontalSize, cardVerticalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QLabel* cardLabel = new QLabel(parent);
    cardLabel->setPixmap(card);

    QLabel* nameLabel = new QLabel(playerName, parent);
    nameLabel->setAlignment(Qt::AlignCenter);

    playerCards[playerName] = cardLabel;
    playerNames[playerName] = nameLabel;
    cardNumbers[playerName] = -1;
    cardState[playerName] = false;
    playerOrder.append(playerName);

    updateCardPositions();

    cardLabel->show();
    nameLabel->show();
}

void CardManager::removeCard(const QString& playerName)
{
    if (!playerCards.contains(playerName)) {
        return;
    }

    QLabel* cardLabel = playerCards.take(playerName);
    delete cardLabel;

    QLabel* nameLabel = playerNames.take(playerName);
    delete nameLabel;

    playerOrder.removeOne(playerName);
    updateCardPositions();
}

void CardManager::showCard(const QString& playerName) {
    std::vector<QString> cardPaintedSymbols = {"0", "1", "2", "3", "5", "8", "13", "21", "34", "55", "89", "?", "☕"};

    if (!playerCards.contains(playerName)) {
        return;
    }

    QLabel* cardLabel = playerCards[playerName];
    int cardNumber = cardNumbers[playerName];
    if (cardNumber == -1) {
        return;
    }

    QPixmap newCard(MainWindow::pathToImages+"2.png");
    newCard = newCard.scaled(cardHorizontalSize, cardVerticalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&newCard);
    painter.setFont(QFont("Arial", 32, QFont::Bold));
    painter.drawText(newCard.rect(), Qt::AlignCenter, cardPaintedSymbols[cardNumber-1]);
    painter.end();

    cardLabel->setPixmap(newCard);
    cardState[playerName] = true;
}

void CardManager::hideCard(const QString& playerName) {
    if (!playerCards.contains(playerName)) {
        return;
    }

    QLabel* cardLabel = playerCards[playerName];
    QPixmap newCard(MainWindow::pathToImages+"1.png");
    newCard = newCard.scaled(cardHorizontalSize, cardVerticalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cardLabel->setPixmap(newCard);
    cardState[playerName] = false;
}

void CardManager::flipCard(const QString& playerName) {
    if (!playerCards.contains(playerName)) {
        return;
    }

    if (cardState[playerName]) {
        hideCard(playerName);
    } else {
        showCard(playerName);
    }
}

void CardManager::flipAllCards(bool faceUp) {
    for (auto it = cardState.begin(); it != cardState.end(); ++it) {
        const QString& playerName = it.key();
        bool currentCardState = it.value();

        if (currentCardState != faceUp) {
            flipCard(playerName);
        }
    }
}

void CardManager::setNumberCard(const QString& playerName, int cardNumber) {
    cardNumbers[playerName] = cardNumber;
}

int CardManager::getNumberCard(const QString& playerName) const {
    if (cardNumbers.contains(playerName)) {
        return cardNumbers[playerName];
    }
    return -1;
}

void CardManager::updateCardPositions()
{

    const int startXPos = 100;
    const int startYPos = 120;
    const int xMargin = 20;
    const int yMargin = 30;

    for (int i = 0; i < playerOrder.size(); ++i) {
        QString playerName = playerOrder[i];

        QLabel* cardLabel = playerCards[playerName];
        QLabel* nameLabel = playerNames[playerName];


        int row = i / 8;
        int col = i % 8;
        int x = startXPos + col * (cardHorizontalSize + xMargin);
        int y = startYPos + row * (cardVerticalSize + yMargin);


        cardLabel->setGeometry(x, y, cardHorizontalSize, cardVerticalSize);


        nameLabel->setGeometry(x, y + cardVerticalSize, cardHorizontalSize, 20);
    }
}

void CardManager::generateChooseCards(QWidget* parent) {

    int startX = 0;
    int endX = 1600;
    int minY = 700;
    int maxY = 900;


    int centerY = minY + (maxY - minY - chooseCardVerticalSize) / 2;


    std::vector<int> cardNumbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    std::vector<QString> cardPaintedSymbols = {"0", "1", "2", "3", "5", "8", "13", "21", "34", "55", "89", "?", "☕"};
    int numCards = static_cast<int>(cardNumbers.size());


    int spacingX = (endX - startX - numCards * chooseCardHorizontalSize) / (numCards + 1);
    if (spacingX < 0) {
        spacingX = 10;
    }

    cards.clear();

    for (int i = 0; i < numCards; ++i) {
        int cardNumber = cardNumbers[i];
        QString cardPaintedSymbol = cardPaintedSymbols[i];
        int posX = startX + spacingX + i * (chooseCardHorizontalSize + spacingX);
        int posY = centerY;


        ClickableLabel* card = new ClickableLabel(parent);


        QPixmap newCard(MainWindow::pathToImages+"2.png");
        newCard = newCard.scaled(chooseCardHorizontalSize, chooseCardVerticalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);


        QPainter painter(&newCard);
        painter.setFont(QFont("Arial", 32, QFont::Bold));
        painter.drawText(newCard.rect(), Qt::AlignCenter, cardPaintedSymbol);
        painter.end();


        card->setPixmap(newCard);


        card->setGeometry(posX, posY, chooseCardHorizontalSize, chooseCardVerticalSize);


        cards.push_back(card);

        QObject::connect(card, &ClickableLabel::clicked, [this, card, cardNumber]() {
            for (ClickableLabel* otherCard : cards) {
                otherCard->setSelected(false);
            }
            card->setSelected(true);
            emit cardClicked(cardNumber);
        });


        card->show();
    }
}

void CardManager::toggleChooseCards(bool enable) {
    for (ClickableLabel* card : cards) {
        card->setEnabled(enable);
        card->setSelected(false);
    }
}



