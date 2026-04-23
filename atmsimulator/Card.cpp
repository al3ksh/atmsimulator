#include "Card.h"
#include "Database.h"

Card::Card()
    : m_id(-1)
    , m_accountId(-1)
    , m_isActive(false)
{
}

Card::Card(int id, const std::string& cardNumber, const std::string& pin, int accountId, bool isActive)
    : m_id(id)
    , m_cardNumber(cardNumber)
    , m_pin(pin)
    , m_accountId(accountId)
    , m_isActive(isActive)
{
}

int Card::getId() const
{
    return m_id;
}

const std::string& Card::getCardNumber() const
{
    return m_cardNumber;
}

const std::string& Card::getPin() const
{
    return m_pin;
}

int Card::getAccountId() const
{
    return m_accountId;
}

bool Card::isActive() const
{
    return m_isActive;
}

bool Card::validatePin(const std::string& pin) const
{
    return m_pin == pin;
}

bool Card::changePin(const std::string& oldPin, const std::string& newPin)
{
    if (oldPin != m_pin || newPin.length() != 4)
    {
        return false;
    }

    auto& db = Database::getInstance();
    if (db.execute("UPDATE cards SET pin = '" + newPin + "' WHERE id = " + std::to_string(m_id)))
    {
        m_pin = newPin;
        return true;
    }
    return false;
}

bool Card::deactivate()
{
    auto& db = Database::getInstance();
    if (db.execute("UPDATE cards SET is_active = 0 WHERE id = " + std::to_string(m_id)))
    {
        m_isActive = false;
        return true;
    }
    return false;
}

Card Card::create(const std::string& cardNumber, const std::string& pin, int accountId)
{
    auto& db = Database::getInstance();
    db.execute("INSERT INTO cards (card_number, pin, account_id) VALUES ('" +
               cardNumber + "', '" + pin + "', " + std::to_string(accountId) + ")");
    int id = db.lastInsertId();
    return load(cardNumber);
}

Card Card::load(const std::string& cardNumber)
{
    auto& db = Database::getInstance();
    auto rows = db.query("SELECT id, card_number, pin, account_id, is_active FROM cards WHERE card_number = '" +
                         cardNumber + "'");

    if (rows.empty())
    {
        return Card();
    }

    return Card(std::stoi(rows[0][0]), rows[0][1], rows[0][2],
                std::stoi(rows[0][3]), rows[0][4] == "1");
}

std::vector<Card> Card::loadAll()
{
    auto& db = Database::getInstance();
    std::vector<Card> cards;

    auto rows = db.query("SELECT id, card_number, pin, account_id, is_active FROM cards ORDER BY id");
    for (const auto& row : rows)
    {
        cards.push_back(Card(std::stoi(row[0]), row[1], row[2],
                             std::stoi(row[3]), row[4] == "1"));
    }

    return cards;
}
