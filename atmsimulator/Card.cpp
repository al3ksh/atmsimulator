#include "Card.h"

Card::Card(const std::string& cardNumber, const std::string& accountId)
    : m_cardNumber(cardNumber)
    , m_accountId(accountId)
{
}

const std::string& Card::getCardNumber() const
{
    return m_cardNumber;
}

const std::string& Card::getAccountId() const
{
    return m_accountId;
}
