# ATM Simulator - Dokumentacja Techniczna

## 1. Cel projektu

Symulator bankomatu (ATM) z interfejsem konsolowym

---

## 2. Architektura systemu

### 2.1. Diagram klas

```
+-------------------+       +-------------------+       +-------------------+
|       ATM         |------>|       Card        |------>|     Account       |
+-------------------+ owns  +-------------------+ refs  +-------------------+
| - m_state         |       | - m_cardNumber    |       | - m_accountId     |
| - m_currentCard   |       | - m_accountId     |       | - m_pin           |
| - m_accounts      |       +-------------------+       | - m_balance       |
| - m_cards         |                                   +-------------------+
+-------------------+                                   | + validatePin()   |
| + run()           |                                   | + withdraw()      |
| + showMainMenu()  |                                   | + deposit()       |
| + handlePinEntry()|                                   | + getBalance()    |
| + handleWithdrawal()                                  +-------------------+
| + handleDeposit() |
| + handleEjectCard()|
+-------------------+
```

### 2.2. Relacje miedzy klasami

- **ATM** jest klasa glowna (orchestrator). Posiada wlascicielekswo wszystkich obiektow `Card` i `Account` przez `std::shared_ptr`.
- **Card** przechowuje numer karty i identyfikator konta (`accountId`) - sluzy jako posrednik miedzy ATM a Account.
- **Account** zarzadza stanem konta: saldem, walidacja PIN i operacjami finansowymi.

### 2.3. Dlaczego taki podzial?

| Decyzja | Uzasadnienie |
|---------|-------------|
| Rozdzielenie Card i Account | W rzeczywistych systemach jedna karta nie musi miec jedno konto (np. wielowalutowe). Oddzielenie pozwala na latwa rozszerzalnosc. |
| `shared_ptr` dla danych | Wiele kart moze wspoldzielic to samo konto. `shared_ptr` zapewnia automatyczne zarzadzanie pamiecia (RAII). |
| Maszyna stanow w ATM | Bankomat ma jasno okreslone stany pracy - FSM zapobiega nielegalnym przejsciom (np. wyplata bez autoryzacji). |
| `unordered_map` jako magazyn | O(1) lookup po numerze karty / ID konta - wydajniejsze niz `map` dla tego przypadku. |

---

## 3. Maszyna stanow (State Machine)

ATM operuje w trzech stanach zdefiniowanych jako `enum class ATMState`:

```
                    +-------+
                    | Idle  |<---------------------------+
                    +-------+                            |
                        |                                |
                   Wloz karte                            |
                        |                                |
                   +------------+   Poprawny PIN    +-----------------+
                   |CardInserted|------------------>| Authenticated   |
                   +------------+                   +-----------------+
                        |                                |
                   Zly PIN                           Menu:
                        |                         [1] Saldo
                   +------------+                 [2] Wyplata
                   |   Idle     |                 [3] Wplata
                   +------------+                 [4] Wyjmij karte -> Idle
```

| Stan | Opis | Dostepne akcje |
|------|------|---------------|
| `Idle` | Ekran powitalny, oczekuje na karte | Wlozenie karty, wyjscie (`q`) |
| `CardInserted` | Karta rozpoznana, czeka na PIN | Weryfikacja PIN (bledny -> powrot do Idle) |
| `Authenticated` | Uzytkownik zweryfikowany | Sprawdz salda, wyplata, wplata, wyjecie karty |

---

## 4. Opis plikow zrodlowych

### `atmsimulator.cpp` (entry point)
Punkt wejscia programu. Tworzy obiekt `ATM` i uruchamia glowna petle.

### `ATM.h` / `ATM.cpp` - Klasa ATM (251 linii)
Orchestrator calosci. Odpowiada za:
- Inicjalizacje danych testowych (`initializeAccounts()`)
- Glowna petle programu z switch-case na stany (`run()`)
- Renderowanie menu i ekranow powitalnych
- Obsluge wejscia uzytkownika (walidacja `std::cin.fail()`)
- Routing operacji do odpowiednich metod

### `Account.h` / `Account.cpp` - Klasa Account (41 linii)
Model konta bankowego. Odpowiada za:
- Przechowywanie ID konta, PIN i salda
- Walidacje PIN (porownanie stringow)
- Wyplate: sprawdza czy kwota > 0 oraz czy saldo wystarcza
- Wplate: akceptuje tylko kwoty dodatnie

### `Card.h` / `Card.cpp` - Klasa Card (17 linii)
Model karty platniczej. Odpowiada za:
- Przechowywanie 16-cyfrowego numeru karty
- Przechowywanie referencji do konta (`accountId`)
- Gettery (const correctness)

---

## 5. Zarzadzanie pamiecia

| Mechanizm | Zastosowanie | Dlaczego |
|-----------|-------------|----------|
| `std::shared_ptr<Account>` | Magazyn kont w ATM | Wielokrotny dostep z roznych punktow (np. przyszle powiazanie wielu kart z jednym kontem) |
| `std::shared_ptr<Card>` | Magazyn kart w ATM | Wspoldzielenie referencji do kart miedzy metoda `findCard()` a `m_currentCard` |
| `const &` w getterach | Wszystkie gettery | Unikniecie kopiowania stringow przy odczycie |
| RAII | Smart pointery automatycznie czyszcza pamiec przy wyjsciu z zasiegu | Brak memory leakow bez recznego `delete` |

---

## 6. Dane testowe

| Card Number | Account ID | PIN | Poczatkowe saldo |
|-------------|-----------|-----|-----------------|
| `1234567890123456` | `ACC001` | `1234` | 1000.00 PLN |
| `9876543210987654` | `ACC002` | `4321` | 500.50 PLN |

Dane inicjalizowane hardcodowo w `ATM::initializeAccounts()`. Brak persystencji - dane znikaja po zakonczeniu programu.

---

## 7. Struktura katalogow

```
atmsim/
└── atmsimulator/
    ├── atmsimulator.slnx              # Solution file (Visual Studio 2022)
    ├── AGENTS.md                      # Guidelines dla AI agentow
    ├── README.md                      # Ten plik
    └── atmsimulator/
        ├── atmsimulator.vcxproj       # Projekt C++
        ├── atmsimulator.cpp           # main() - punkt wejscia
        ├── ATM.h / ATM.cpp            # Orchestrator + maszyna stanow
        ├── Account.h / Account.cpp    # Model konta bankowego
        ├── Card.h / Card.cpp          # Model karty platniczej
        └── x64/Debug/
            └── atmsimulator.exe       # Skompilowany binarka
```
