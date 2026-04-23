#pragma once

#include <string>

class Card
{
public:
    Card(const std::string& cardNumber, const std::string& accountId);

    const std::string& getCardNumber() const;
    const std::string& getAccountId() const;

private:
    std::string m_cardNumber;
    std::string m_accountId;
};
