#pragma once

#include <string>

#include "TransactionType.h"

class Transaction
{
public:
    Transaction(int id, int fromAccountId, int toAccountId, TransactionType type,
                double amount, const std::string& currency, const std::string& timestamp);

    int getId() const;
    int getFromAccountId() const;
    int getToAccountId() const;
    TransactionType getType() const;
    double getAmount() const;
    const std::string& getCurrency() const;
    const std::string& getTimestamp() const;

    static int createInDb(int fromAccountId, int toAccountId, TransactionType type,
                          double amount, const std::string& currency);

private:
    Transaction();

    int m_id;
    int m_fromAccountId;
    int m_toAccountId;
    TransactionType m_type;
    double m_amount;
    std::string m_currency;
    std::string m_timestamp;
};
