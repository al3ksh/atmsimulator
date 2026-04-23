#pragma once

#include <string>
#include <vector>

class Card
{
public:
    Card();

    int getId() const;
    const std::string& getCardNumber() const;
    const std::string& getPin() const;
    int getAccountId() const;
    bool isActive() const;

    bool validatePin(const std::string& pin) const;
    bool changePin(const std::string& oldPin, const std::string& newPin);
    bool deactivate();

    static Card create(const std::string& cardNumber, const std::string& pin, int accountId);
    static Card load(const std::string& cardNumber);
    static std::vector<Card> loadAll();

private:
    Card(int id, const std::string& cardNumber, const std::string& pin, int accountId, bool isActive);

    int m_id;
    std::string m_cardNumber;
    std::string m_pin;
    int m_accountId;
    bool m_isActive;
};
