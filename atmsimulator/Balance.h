#pragma once

#include <string>

class Balance
{
public:
    Balance(int id, int accountId, const std::string& currency, double amount);

    int getId() const;
    int getAccountId() const;
    const std::string& getCurrency() const;
    double getAmount() const;

    void setAmount(double amount);
    bool saveToDb();

private:
    int m_id;
    int m_accountId;
    std::string m_currency;
    double m_amount;
};
