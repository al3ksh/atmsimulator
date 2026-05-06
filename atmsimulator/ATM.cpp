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
        case ATMState::AdminMenu:
            showAdminMenu();
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
    std::cout << "5. Historia transakcji\n";
    std::cout << "6. Wyjmij karte\n";
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
        handleTransactionHistory();
        break;
    case 6:
        handleEjectCard();
        break;
    default:
        std::cout << "Nieprawidlowa opcja. Wybierz 1-6.\n";
        break;
    }
}

void ATM::handleCardInsertion()
{
    std::cout << "Podaj numer karty (lub 'q' aby wyjsc, 'a' aby wejsc do panelu admina): ";
    std::string input;
    std::cin >> input;

    if (input == "q" || input == "Q")
    {
        std::cout << "Dziekujemy za skorzystanie z bankomatu. Do widzenia!\n";
        exit(0);
    }

    if (input == "a" || input == "A")
    {
        handleAdminLogin();
        return;
    }

    Card* card = findCard(input);
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

void ATM::handleTransactionHistory()
{
    Account* account = getCurrentAccount();
    if (!account)
    {
        return;
    }

    auto& db = Database::getInstance();
    auto rows = db.query("SELECT type, amount, currency, timestamp FROM transactions "
                         "WHERE from_account_id = " + std::to_string(account->getId()) +
                         " OR to_account_id = " + std::to_string(account->getId()) +
                         " ORDER BY id DESC LIMIT 20");

    std::cout << "\n========================================\n";
    std::cout << "HISTORIA TRANSAKCJI - " << account->getOwnerName() << "\n";
    std::cout << "========================================\n";

    if (rows.empty())
    {
        std::cout << "Brak transakcji.\n";
    }
    else
    {
        for (const auto& row : rows)
        {
            std::cout << row[3] << " | " << row[0] << " | "
                      << std::fixed << std::setprecision(2) << std::stod(row[1])
                      << " " << row[2] << "\n";
        }
    }

    std::cout << "========================================\n";
}

void ATM::handleAdminLogin()
{
    std::cout << "\n--- PANEL ADMINA ---\n";
    std::cout << "Login: ";
    std::string login;
    std::cin >> login;

    std::cout << "Haslo: ";
    std::string password;
    std::cin >> password;

    if (login == "admin" && password == "admin")
    {
        m_state = ATMState::AdminMenu;
        std::cout << "Zalogowano jako admin.\n";
    }
    else
    {
        std::cout << "Nieprawidlowy login lub haslo.\n";
    }
}

void ATM::showAdminMenu()
{
    std::cout << "\n";
    std::cout << "========== PANEL ADMINA ==========\n";
    std::cout << "1. Utworz nowe konto\n";
    std::cout << "2. Usun konto\n";
    std::cout << "3. Wyswietl wszystkie konta\n";
    std::cout << "4. Dodaj karte do konta\n";
    std::cout << "5. Wyswietl wszystkie karty\n";
    std::cout << "6. Ustaw saldo konta\n";
    std::cout << "0. Wroc do ekranu glownego\n";
    std::cout << "\n";
    std::cout << "Wybierz opcje: ";

    int choice;
    std::cin >> choice;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowy wybor.\n";
        return;
    }

    switch (choice)
    {
    case 1:
        adminCreateAccount();
        break;
    case 2:
        adminDeleteAccount();
        break;
    case 3:
        adminListAccounts();
        break;
    case 4:
        adminAddCard();
        break;
    case 5:
        adminListCards();
        break;
    case 6:
        adminSetBalance();
        break;
    case 0:
        m_state = ATMState::Idle;
        std::cout << "Wylogowano z panelu admina.\n";
        break;
    default:
        std::cout << "Nieprawidlowa opcja. Wybierz 0-6.\n";
        break;
    }
}

void ATM::adminCreateAccount()
{
    std::cout << "Podaj imie i nazwisko wlasciciela: ";
    std::string name;
    std::cin.ignore();
    std::getline(std::cin, name);

    auto account = Account::create(name);
    m_accounts[account.getId()] = account;
    std::cout << "Konto utworzone. ID: " << account.getId() << "\n";
}

void ATM::adminDeleteAccount()
{
    std::cout << "Podaj ID konta do usuniecia: ";
    int id;
    std::cin >> id;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowe ID.\n";
        return;
    }

    auto& db = Database::getInstance();

    auto cards = db.query("SELECT card_number FROM cards WHERE account_id = " + std::to_string(id));
    for (const auto& row : cards)
    {
        m_cards.erase(row[0]);
    }

    if (db.execute("DELETE FROM balances WHERE account_id = " + std::to_string(id)) &&
        db.execute("DELETE FROM cards WHERE account_id = " + std::to_string(id)) &&
        db.execute("DELETE FROM transactions WHERE from_account_id = " + std::to_string(id) +
                   " OR to_account_id = " + std::to_string(id)) &&
        db.execute("DELETE FROM accounts WHERE id = " + std::to_string(id)))
    {
        m_accounts.erase(id);
        std::cout << "Konto " << id << " zostalo usuniete.\n";
    }
    else
    {
        std::cout << "Nie udalo sie usunac konta.\n";
    }
}

void ATM::adminListAccounts()
{
    std::cout << "\n--- LISTA KONT ---\n";

    for (const auto& [id, account] : m_accounts)
    {
        std::cout << "ID: " << id << " | " << account.getOwnerName() << "\n";
        for (const auto& bal : account.getBalances())
        {
            std::cout << "  " << bal.getCurrency() << ": "
                      << std::fixed << std::setprecision(2) << bal.getAmount() << "\n";
        }
    }

    if (m_accounts.empty())
    {
        std::cout << "Brak kont.\n";
    }
}

void ATM::adminAddCard()
{
    adminListAccounts();

    std::cout << "\nPodaj ID konta: ";
    int accountId;
    std::cin >> accountId;

    if (std::cin.fail() || m_accounts.find(accountId) == m_accounts.end())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowe ID konta.\n";
        return;
    }

    std::cout << "Podaj numer karty (16 cyfr): ";
    std::string cardNumber;
    std::cin >> cardNumber;

    std::cout << "Podaj PIN (4 cyfry): ";
    std::string pin;
    std::cin >> pin;

    auto card = Card::create(cardNumber, pin, accountId);
    if (card.getId() > 0)
    {
        m_cards[card.getCardNumber()] = card;
        std::cout << "Karta dodana. Numer: " << card.getCardNumber() << "\n";
    }
    else
    {
        std::cout << "Nie udalo sie dodac karty. Moze numer juz istnieje?\n";
    }
}

void ATM::adminListCards()
{
    std::cout << "\n--- LISTA KART ---\n";

    for (const auto& [num, card] : m_cards)
    {
        std::cout << "Numer: " << card.getCardNumber()
                  << " | Konto ID: " << card.getAccountId()
                  << " | Aktywna: " << (card.isActive() ? "Tak" : "Nie") << "\n";
    }

    if (m_cards.empty())
    {
        std::cout << "Brak kart.\n";
    }
}

void ATM::adminSetBalance()
{
    adminListAccounts();

    std::cout << "\nPodaj ID konta: ";
    int accountId;
    std::cin >> accountId;

    if (std::cin.fail() || m_accounts.find(accountId) == m_accounts.end())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowe ID konta.\n";
        return;
    }

    std::cout << "Podaj walute (np. PLN): ";
    std::string currency;
    std::cin >> currency;

    std::cout << "Podaj nowa kwote: ";
    double amount;
    std::cin >> amount;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowa kwota.\n";
        return;
    }

    auto& account = m_accounts[accountId];
    auto bal = account.getBalance(currency);
    if (bal)
    {
        bal->setAmount(amount);
        bal->saveToDb();
        m_accounts[accountId] = account;
        std::cout << "Saldo ustawione: " << std::fixed << std::setprecision(2) << amount << " " << currency << "\n";
    }
    else
    {
        std::cout << "Nie znaleziono salda w walucie " << currency << " dla tego konta.\n";
    }
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
