#pragma once

#include <string>
#include <unordered_map>

#include "Account.h"
#include "Card.h"

class ATM
{
public:
    ATM();
    ~ATM() = default;

    void run();

private:
    enum class ATMState
    {
        Idle,
        CardInserted,
        Authenticated,
        AdminMenu
    };

    void showWelcomeScreen();
    void showMainMenu();
    void handleCardInsertion();
    void handlePinEntry();
    void handleBalanceInquiry();
    void handleWithdrawal();
    void handleDeposit();
    void handleEjectCard();
    void handleTransactionHistory();
    void handleTransfer();

    void handleAdminLogin();
    void showAdminMenu();
    void adminCreateAccount();
    void adminDeleteAccount();
    void adminListAccounts();
    void adminAddCard();
    void adminListCards();
    void adminSetBalance();

    Card* findCard(const std::string& cardNumber);
    Account* getCurrentAccount();

    ATMState m_state;
    Card m_currentCard;
    std::unordered_map<int, Account> m_accounts;
    std::unordered_map<std::string, Card> m_cards;
};
