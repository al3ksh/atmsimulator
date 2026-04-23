#pragma once

#include <string>

enum class TransactionType
{
    Deposit,
    Withdrawal,
    Transfer
};

namespace TransactionTypeUtils
{
    std::string toString(TransactionType type);
    TransactionType fromString(const std::string& str);
}
