#include "ATM.h"

#include <iostream>
#include <iomanip>
#include <limits>

ATM::ATM()
    : m_state(ATMState::Idle)
    , m_currentCard(nullptr)
{
    initializeAccounts();
}

void ATM::initializeAccounts()
{
    auto account1 = std::make_shared<Account>("ACC001", "1234", 1000.00);
    auto account2 = std::make_shared<Account>("ACC002", "4321", 500.50);

    m_accounts[account1->getAccountId()] = account1;
    m_accounts[account2->getAccountId()] = account2;

    auto card1 = std::make_shared<Card>("1234567890123456", "ACC001");
    auto card2 = std::make_shared<Card>("9876543210987654", "ACC002");

    m_cards[card1->getCardNumber()] = card1;
    m_cards[card2->getCardNumber()] = card2;
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
    std::cout << "4. Wyjmij karte\n";
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
        handleEjectCard();
        break;
    default:
        std::cout << "Nieprawidlowa opcja. Wybierz 1-4.\n";
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

    m_currentCard = findCard(cardNumber);
    if (m_currentCard)
    {
        m_state = ATMState::CardInserted;
        std::cout << "Karta zaakceptowana.\n";
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

    auto account = getCurrentAccount();
    if (account && account->validatePin(pin))
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
    auto account = getCurrentAccount();
    if (account)
    {
        std::cout << "\n========================================\n";
        std::cout << "STAN KONTA\n";
        std::cout << "========================================\n";
        std::cout << "Dostepne srodki: " << std::fixed << std::setprecision(2)
                  << account->getBalance() << " PLN\n";
        std::cout << "========================================\n";
    }
}

void ATM::handleWithdrawal()
{
    std::cout << "\nPodaj kwote do wyplaty: ";
    double amount;
    std::cin >> amount;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowa kwota.\n";
        return;
    }

    auto account = getCurrentAccount();
    if (account)
    {
        if (account->withdraw(amount))
        {
            std::cout << "Wyplata: " << std::fixed << std::setprecision(2)
                      << amount << " PLN\n";
            std::cout << "Odbierz gotowke.\n";
            std::cout << "Nowe saldo: " << account->getBalance() << " PLN\n";
        }
        else
        {
            std::cout << "Nie mozna wykonac wyplaty. Sprawdz kwote lub saldo.\n";
        }
    }
}

void ATM::handleDeposit()
{
    std::cout << "\nPodaj kwote do wplaty: ";
    double amount;
    std::cin >> amount;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nieprawidlowa kwota.\n";
        return;
    }

    auto account = getCurrentAccount();
    if (account)
    {
        if (amount > 0)
        {
            account->deposit(amount);
            std::cout << "Wplata: " << std::fixed << std::setprecision(2)
                      << amount << " PLN\n";
            std::cout << "Nowe saldo: " << account->getBalance() << " PLN\n";
        }
        else
        {
            std::cout << "Kwota musi byc dodatnia.\n";
        }
    }
}

void ATM::handleEjectCard()
{
    std::cout << "\nKarta zostala wyjeta.\n";
    m_currentCard = nullptr;
    m_state = ATMState::Idle;
}

std::shared_ptr<Card> ATM::findCard(const std::string& cardNumber) const
{
    auto it = m_cards.find(cardNumber);
    if (it != m_cards.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Account> ATM::getCurrentAccount() const
{
    if (m_currentCard)
    {
        auto it = m_accounts.find(m_currentCard->getAccountId());
        if (it != m_accounts.end())
        {
            return it->second;
        }
    }
    return nullptr;
}
