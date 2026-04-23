#include "Transaction.h"
#include "Database.h"

Transaction::Transaction()
    : m_id(-1)
    , m_fromAccountId(-1)
    , m_toAccountId(-1)
    , m_type(TransactionType::Deposit)
    , m_amount(0.0)
{
}

Transaction::Transaction(int id, int fromAccountId, int toAccountId, TransactionType type,
                         double amount, const std::string& currency, const std::string& timestamp)
    : m_id(id)
    , m_fromAccountId(fromAccountId)
    , m_toAccountId(toAccountId)
    , m_type(type)
    , m_amount(amount)
    , m_currency(currency)
    , m_timestamp(timestamp)
{
}

int Transaction::getId() const
{
    return m_id;
}

int Transaction::getFromAccountId() const
{
    return m_fromAccountId;
}

int Transaction::getToAccountId() const
{
    return m_toAccountId;
}

TransactionType Transaction::getType() const
{
    return m_type;
}

double Transaction::getAmount() const
{
    return m_amount;
}

const std::string& Transaction::getCurrency() const
{
    return m_currency;
}

const std::string& Transaction::getTimestamp() const
{
    return m_timestamp;
}

int Transaction::createInDb(int fromAccountId, int toAccountId, TransactionType type,
                             double amount, const std::string& currency)
{
    auto& db = Database::getInstance();

    std::string from = (fromAccountId > 0) ? std::to_string(fromAccountId) : "NULL";
    std::string to = (toAccountId > 0) ? std::to_string(toAccountId) : "NULL";

    db.execute("INSERT INTO transactions (from_account_id, to_account_id, type, amount, currency) "
               "VALUES (" + from + ", " + to + ", '" +
               TransactionTypeUtils::toString(type) + "', " +
               std::to_string(amount) + ", '" + currency + "')");

    return db.lastInsertId();
}
