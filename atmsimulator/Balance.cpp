#include "Balance.h"
#include "Database.h"

Balance::Balance(int id, int accountId, const std::string& currency, double amount)
    : m_id(id)
    , m_accountId(accountId)
    , m_currency(currency)
    , m_amount(amount)
{
}

int Balance::getId() const
{
    return m_id;
}

int Balance::getAccountId() const
{
    return m_accountId;
}

const std::string& Balance::getCurrency() const
{
    return m_currency;
}

double Balance::getAmount() const
{
    return m_amount;
}

void Balance::setAmount(double amount)
{
    m_amount = amount;
}

bool Balance::saveToDb()
{
    auto& db = Database::getInstance();
    return db.execute("UPDATE balances SET amount = " + std::to_string(m_amount) +
                      " WHERE id = " + std::to_string(m_id));
}
