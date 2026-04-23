#pragma once

#include <memory>
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
        Authenticated
    };

    void initializeAccounts();
    void showWelcomeScreen();
    void showMainMenu();
    void handleCardInsertion();
    void handlePinEntry();
    void handleBalanceInquiry();
    void handleWithdrawal();
    void handleDeposit();
    void handleEjectCard();

    std::shared_ptr<Card> findCard(const std::string& cardNumber) const;
    std::shared_ptr<Account> getCurrentAccount() const;

    ATMState m_state;
    std::shared_ptr<Card> m_currentCard;
    std::unordered_map<std::string, std::shared_ptr<Account>> m_accounts;
    std::unordered_map<std::string, std::shared_ptr<Card>> m_cards;
};
