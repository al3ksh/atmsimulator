#pragma once

#include <string>

class Account
{
public:
    Account(const std::string& accountId, const std::string& pin, double initialBalance);

    const std::string& getAccountId() const;
    double getBalance() const;

    bool validatePin(const std::string& pin) const;
    bool withdraw(double amount);
    void deposit(double amount);

private:
    std::string m_accountId;
    std::string m_pin;
    double m_balance;
};
