#include "Account.h"

Account::Account(const std::string& accountId, const std::string& pin, double initialBalance)
    : m_accountId(accountId)
    , m_pin(pin)
    , m_balance(initialBalance)
{
}

const std::string& Account::getAccountId() const
{
    return m_accountId;
}

double Account::getBalance() const
{
    return m_balance;
}

bool Account::validatePin(const std::string& pin) const
{
    return m_pin == pin;
}

bool Account::withdraw(double amount)
{
    if (amount <= 0 || amount > m_balance)
    {
        return false;
    }
    m_balance -= amount;
    return true;
}

void Account::deposit(double amount)
{
    if (amount > 0)
    {
        m_balance += amount;
    }
}
