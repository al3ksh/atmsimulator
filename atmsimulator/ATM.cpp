#include "ATM.h"
#include "Database.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>

ATM::ATM()
    : m_state(ATMState::Idle)
{
    auto& db = Database::getInstance();

    std::filesystem::path dbPath = "atm.db";
    if (!db.open(dbPath.string()))
    {
        std::cerr << "Blad krytyczny: nie udalo sie otworzyc bazy danych.\n";
        exit(1);
    }

    db.createTables();

    auto countRows = db.query("SELECT COUNT(*) FROM accounts");
    if (countRows.empty() || countRows[0][0] == "0")
    {
        std::cout << "Inicjalizacja danych testowych...\n";

        auto acc1 = Account::create("Jan Kowalski");
        Card::create("1234567890123456", "1234", acc1.getId());
        auto bal1 = acc1.getBalance("PLN");
        if (bal1)
        {
            bal1->setAmount(1000.00);
            bal1->saveToDb();
        }

        auto acc2 = Account::create("Anna Nowak");
        Card::create("9876543210987654", "4321", acc2.getId());
        auto bal2 = acc2.getBalance("PLN");
        if (bal2)
        {
            bal2->setAmount(500.50);
            bal2->saveToDb();
        }

        std::cout << "Dane testowe wgrane.\n";
    }

    for (const auto& account : Account::loadAll())
    {
        m_accounts[account.getId()] = account;
    }

    for (const auto& card : Card::loadAll())
    {
        m_cards[card.getCardNumber()] = card;
    }
}

void ATM::run()
{
    while (true)
    {
        switch (m_state)
        {
        case ATMState::Idle:
            showWelcomeScreen();
            handleCardInsertion();
            break;
        case ATMState::CardInserted:
            handlePinEntry();
            break;
        case ATMState::Authenticated:
            showMainMenu();
            break;
        }
    }
}

void ATM::showWelcomeScreen()
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           WITAMY W BANKOMACIE          \n";
    std::cout << "========================================\n";
    std::cout << "\n";
    std::cout << "Dostepne karty testowe:\n";
    std::cout << "  1. 1234567890123456 (PIN: 1234)\n";
    std::cout << "  2. 9876543210987654 (PIN: 4321)\n";
    std::cout << "\n";
}

void ATM::showMainMenu()
{
    std::cout << "\n";
    std::cout << "------------- MENU GLOWNE -------------\n";
    std::cout << "1. Sprawdz saldo\n";
    std::cout << "2. Wyplata gotowki\n";
    std::cout << "3. Wplata gotowki\n";
    std::cout << "4. Zmien PIN\n";
    std::cout << "5. Wyjmij karte\n";
    std::cout << "\n";
    std::cout << "Wybierz opcje: ";

    int choice;
    std::cin >> choice;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowy wybor. Sprobuj ponownie.\n";
        return;
    }

    switch (choice)
    {
    case 1:
        handleBalanceInquiry();
        break;
    case 2:
        handleWithdrawal();
        break;
    case 3:
        handleDeposit();
        break;
    case 4:
    {
        std::string oldPin, newPin;
        std::cout << "Podaj obecny PIN: ";
        std::cin >> oldPin;
        std::cout << "Podaj nowy PIN (4 cyfry): ";
        std::cin >> newPin;

        if (m_currentCard.changePin(oldPin, newPin))
        {
            m_cards[m_currentCard.getCardNumber()] = m_currentCard;
            std::cout << "PIN zostal zmieniony.\n";
        }
        else
        {
            std::cout << "Nie udalo sie zmienic PIN. Sprawdz obecny PIN.\n";
        }
        break;
    }
    case 5:
        handleEjectCard();
        break;
    default:
        std::cout << "Nieprawidlowa opcja. Wybierz 1-5.\n";
        break;
    }
}

void ATM::handleCardInsertion()
{
    std::cout << "Podaj numer karty (lub 'q' aby wyjsc): ";
    std::string cardNumber;
    std::cin >> cardNumber;

    if (cardNumber == "q" || cardNumber == "Q")
    {
        std::cout << "Dziekujemy za skorzystanie z bankomatu. Do widzenia!\n";
        exit(0);
    }

    Card* card = findCard(cardNumber);
    if (card && card->isActive())
    {
        m_currentCard = *card;
        m_state = ATMState::CardInserted;
        std::cout << "Karta zaakceptowana.\n";
    }
    else if (card)
    {
        std::cout << "Karta jest zablokowana.\n";
    }
    else
    {
        std::cout << "Nieznana karta. Sprobuj ponownie.\n";
    }
}

void ATM::handlePinEntry()
{
    std::cout << "\nPodaj PIN: ";
    std::string pin;
    std::cin >> pin;

    if (m_currentCard.validatePin(pin))
    {
        m_state = ATMState::Authenticated;
        std::cout << "Autoryzacja pomyslna.\n";
    }
    else
    {
        std::cout << "Nieprawidlowy PIN.\n";
        handleEjectCard();
    }
}

void ATM::handleBalanceInquiry()
{
    Account* account = getCurrentAccount();
    if (account)
    {
        std::cout << "\n========================================\n";
        std::cout << "STAN KONTA - " << account->getOwnerName() << "\n";
        std::cout << "========================================\n";
        for (const auto& bal : account->getBalances())
        {
            std::cout << "  " << bal.getCurrency() << ": "
                      << std::fixed << std::setprecision(2) << bal.getAmount() << "\n";
        }
        std::cout << "========================================\n";
    }
}

void ATM::handleWithdrawal()
{
    Account* account = getCurrentAccount();
    if (!account)
    {
        return;
    }

    std::cout << "Podaj walute (np. PLN): ";
    std::string currency;
    std::cin >> currency;

    std::cout << "Podaj kwote do wyplaty: ";
    double amount;
    std::cin >> amount;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowa kwota.\n";
        return;
    }

    if (account->withdraw(currency, amount))
    {
        m_accounts[account->getId()] = *account;
        std::cout << "Wyplata: " << std::fixed << std::setprecision(2) << amount << " " << currency << "\n";
        std::cout << "Odbierz gotowke.\n";
        auto bal = account->getBalance(currency);
        if (bal)
        {
            std::cout << "Nowe saldo: " << bal->getAmount() << " " << currency << "\n";
        }
    }
    else
    {
        std::cout << "Nie mozna wykonac wyplaty. Sprawdz kwote lub saldo.\n";
    }
}

void ATM::handleDeposit()
{
    Account* account = getCurrentAccount();
    if (!account)
    {
        return;
    }

    std::cout << "Podaj walute (np. PLN): ";
    std::string currency;
    std::cin >> currency;

    std::cout << "Podaj kwote do wplaty: ";
    double amount;
    std::cin >> amount;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowa kwota.\n";
        return;
    }

    if (account->deposit(currency, amount))
    {
        m_accounts[account->getId()] = *account;
        std::cout << "Wplata: " << std::fixed << std::setprecision(2) << amount << " " << currency << "\n";
        auto bal = account->getBalance(currency);
        if (bal)
        {
            std::cout << "Nowe saldo: " << bal->getAmount() << " " << currency << "\n";
        }
    }
    else
    {
        std::cout << "Nie udalo sie wykonac wplaty.\n";
    }
}

void ATM::handleEjectCard()
{
    std::cout << "\nKarta zostala wyjeta.\n";
    m_currentCard = Card();
    m_state = ATMState::Idle;
}

Card* ATM::findCard(const std::string& cardNumber)
{
    auto it = m_cards.find(cardNumber);
    if (it != m_cards.end())
    {
        return &it->second;
    }
    return nullptr;
}

Account* ATM::getCurrentAccount()
{
    if (m_currentCard.getId() < 0)
    {
        return nullptr;
    }

    auto it = m_accounts.find(m_currentCard.getAccountId());
    if (it != m_accounts.end())
    {
        return &it->second;
    }
    return nullptr;
}
