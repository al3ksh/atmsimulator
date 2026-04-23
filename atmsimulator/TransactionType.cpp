#include "TransactionType.h"

std::string TransactionTypeUtils::toString(TransactionType type)
{
    switch (type)
    {
    case TransactionType::Deposit:
        return "DEPOSIT";
    case TransactionType::Withdrawal:
        return "WITHDRAWAL";
    case TransactionType::Transfer:
        return "TRANSFER";
    }
    return "UNKNOWN";
}

TransactionType TransactionTypeUtils::fromString(const std::string& str)
{
    if (str == "DEPOSIT")
    {
        return TransactionType::Deposit;
    }
    if (str == "WITHDRAWAL")
    {
        return TransactionType::Withdrawal;
    }
    if (str == "TRANSFER")
    {
        return TransactionType::Transfer;
    }
    return TransactionType::Deposit;
}
