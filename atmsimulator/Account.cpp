#include "Account.h"
#include "Database.h"

Account::Account()
    : m_id(-1)
{
}

Account::Account(int id, const std::string& ownerName, const std::string& createdAt)
    : m_id(id)
    , m_ownerName(ownerName)
    , m_createdAt(createdAt)
{
}

int Account::getId() const
{
    return m_id;
}

const std::string& Account::getOwnerName() const
{
    return m_ownerName;
}

const std::string& Account::getCreatedAt() const
{
    return m_createdAt;
}

const std::vector<Balance>& Account::getBalances() const
{
    return m_balances;
}

Balance* Account::getBalance(const std::string& currency)
{
    for (auto& bal : m_balances)
    {
        if (bal.getCurrency() == currency)
        {
            return &bal;
        }
    }
    return nullptr;
}

bool Account::deposit(const std::string& currency, double amount)
{
    if (amount <= 0)
    {
        return false;
    }

    auto bal = getBalance(currency);
    if (!bal)
    {
        return false;
    }

    bal->setAmount(bal->getAmount() + amount);
    Transaction::createInDb(m_id, -1, TransactionType::Deposit, amount, currency);
    return bal->saveToDb();
}

bool Account::withdraw(const std::string& currency, double amount)
{
    if (amount <= 0)
    {
        return false;
    }

    auto bal = getBalance(currency);
    if (!bal || amount > bal->getAmount())
    {
        return false;
    }

    bal->setAmount(bal->getAmount() - amount);
    Transaction::createInDb(m_id, -1, TransactionType::Withdrawal, amount, currency);
    return bal->saveToDb();
}

Account Account::create(const std::string& ownerName)
{
    auto& db = Database::getInstance();
    db.execute("INSERT INTO accounts (owner_name) VALUES ('" + ownerName + "')");
    int id = db.lastInsertId();
    db.execute("INSERT INTO balances (account_id, currency, amount) VALUES (" +
               std::to_string(id) + ", 'PLN', 0.0)");
    return load(id);
}

Account Account::load(int accountId)
{
    auto& db = Database::getInstance();
    auto rows = db.query("SELECT id, owner_name, created_at FROM accounts WHERE id = " +
                         std::to_string(accountId));

    if (rows.empty())
    {
        return Account();
    }

    Account acc(std::stoi(rows[0][0]), rows[0][1], rows[0][2]);

    auto balRows = db.query("SELECT id, account_id, currency, amount FROM balances WHERE account_id = " +
                            std::to_string(accountId));
    for (const auto& row : balRows)
    {
        acc.m_balances.push_back(Balance(std::stoi(row[0]), std::stoi(row[1]), row[2], std::stod(row[3])));
    }

    return acc;
}

std::vector<Account> Account::loadAll()
{
    auto& db = Database::getInstance();
    std::vector<Account> accounts;

    auto rows = db.query("SELECT id, owner_name, created_at FROM accounts ORDER BY id");
    for (const auto& row : rows)
    {
        Account acc(std::stoi(row[0]), row[1], row[2]);

        auto balRows = db.query("SELECT id, account_id, currency, amount FROM balances WHERE account_id = " +
                                row[0]);
        for (const auto& balRow : balRows)
        {
            acc.m_balances.push_back(Balance(std::stoi(balRow[0]), std::stoi(balRow[1]),
                                              balRow[2], std::stod(balRow[3])));
        }

        accounts.push_back(acc);
    }

    return accounts;
}
