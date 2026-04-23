#pragma once

#include <string>
#include <vector>

#include "Balance.h"
#include "Transaction.h"

class Account
{
public:
    Account();

    int getId() const;
    const std::string& getOwnerName() const;
    const std::string& getCreatedAt() const;

    const std::vector<Balance>& getBalances() const;
    Balance* getBalance(const std::string& currency);

    bool deposit(const std::string& currency, double amount);
    bool withdraw(const std::string& currency, double amount);

    static Account create(const std::string& ownerName);
    static Account load(int accountId);
    static std::vector<Account> loadAll();

private:
    Account(int id, const std::string& ownerName, const std::string& createdAt);

    int m_id;
    std::string m_ownerName;
    std::string m_createdAt;
    std::vector<Balance> m_balances;
};
