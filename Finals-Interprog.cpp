#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <limits>
#include <sstream>
using namespace std;

// -------- Exception Handling --------
class ReservationException : public exception {
    string message;
public:
    ReservationException(const string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

// -------- Reservation Struct --------
struct Reservation {
    int id;
    string customerName;
    string phoneNumber;
    int partySize;
    string date;
    string time;
    int tableNumber;

    Reservation(int id, const string& name, const string& phone, int size, const string& date, const string& time, int table)
        : id(id), customerName(name), phoneNumber(phone), partySize(size), date(date), time(time), tableNumber(table) {}
};

// -------- Strategy Pattern --------
class TableAssignmentStrategy {
public:
    virtual int assignTable(const vector<bool>& tables) = 0;
    virtual ~TableAssignmentStrategy() = default;
};

class AutoAssignTable : public TableAssignmentStrategy {
public:
    int assignTable(const vector<bool>& tables) override {
        for (int i = 0; i < tables.size(); ++i) {
            if (tables[i]) return i;
        }
        throw ReservationException("No available tables.");
    }
};

// -------- Singleton Pattern --------
class ReservationManager {
private:
    vector<bool> tables;
    vector<Reservation> reservations;
    static unique_ptr<ReservationManager> instance;
    int nextReservationId;
    ReservationManager() : tables(10, true), nextReservationId(1) {} // 10 tables
public:
    static ReservationManager& getInstance() {
        if (!instance)
            instance.reset(new ReservationManager());
        return *instance;
    }

    void viewTableAvailability() {
        for (int i = 0; i < tables.size(); ++i) {
            cout << "Table " << i + 1 << " is " << (tables[i] ? "AVAILABLE" : "BOOKED") << endl;
        }
    }

    int reserveTable(shared_ptr<TableAssignmentStrategy> strategy, const string& customerName, const string& phoneNumber, 
                    int partySize, const string& date, const string& time) {
        int tableIndex = strategy->assignTable(tables);
        tables[tableIndex] = false;
        int reservationId = nextReservationId++;
        reservations.emplace_back(reservationId, customerName, phoneNumber, partySize, date, time, tableIndex);
        return tableIndex;
    }

    void cancelReservation(int tableIndex, const string& customerName) {
        if (tableIndex < 0 || tableIndex >= tables.size()) {
            throw ReservationException("Invalid table index.");
        }
        tables[tableIndex] = true;
        // Mark reservation as cancelled by removing it
        auto it = reservations.begin();
        while (it != reservations.end()) {
            if (it->customerName == customerName && it->tableNumber == tableIndex) {
                it = reservations.erase(it);
            } else {
                ++it;
            }
        }
    }

    void viewCustomerReservations(const string& customerName) {
        cout << "\n--- Your Reservations ---\n";
        bool hasReservations = false;
        for (const auto& res : reservations) {
            if (res.customerName == customerName) {
                cout << "ID: " << res.id << ", Name: " << res.customerName 
                     << ", Contact: " << res.phoneNumber << ", Party Size: " << res.partySize 
                     << ", Date: " << res.date << ", Time: " << res.time 
                     << ", Table: " << res.tableNumber + 1 << endl;
                hasReservations = true;
            }
        }
        if (!hasReservations) {
            cout << "No active reservations found for " << customerName << ".\n";
        }
    }

    void updateReservation(int oldTableIndex, const string& customerName, shared_ptr<TableAssignmentStrategy> strategy) {
        if (oldTableIndex < 0 || oldTableIndex >= tables.size()) {
            throw ReservationException("Invalid table index.");
        }
        if (tables[oldTableIndex]) {
            throw ReservationException("Table is not currently booked by you.");
        }
        int newTableIndex = strategy->assignTable(tables);
        tables[oldTableIndex] = true;
        tables[newTableIndex] = false;
        // Update the reservation
        for (auto& res : reservations) {
            if (res.customerName == customerName && res.tableNumber == oldTableIndex) {
                res.tableNumber = newTableIndex;
                break;
            }
        }
    }

    void viewLogs() {
        cout << "\n--- Reservation Logs ---\n";
        for (const auto& res : reservations) {
            cout << "ID: " << res.id << ", Name: " << res.customerName 
                 << ", Contact: " << res.phoneNumber << ", Party Size: " << res.partySize 
                 << ", Date: " << res.date << ", Time: " << res.time 
                 << ", Table: " << res.tableNumber + 1 << endl;
        }
    }
};

unique_ptr<ReservationManager> ReservationManager::instance = nullptr;

// -------- Abstraction + Polymorphism --------
class User {
protected:
    string username;
public:
    User(const string& name) : username(name) {}
    virtual void showMenu() = 0;
    virtual ~User() = default;
};

// Account database
map<string, string> receptionistAccounts;
map<string, string> customerAccounts;

// -------- Inheritance for Roles --------
class Customer : public User {
public:
    Customer(const string& name) : User(name) {}
    void showMenu() override {
        bool isRunning = true;
        while (isRunning) {
            int choice;
            cout << "\n[Customer Menu - " << username << "]\n";
            cout << "1. View My Reservations\n";
            cout << "2. Reserve Table\n";
            cout << "3. View Availability\n";
            cout << "4. Update Reservation\n";
            cout << "5. Cancel Reservation\n";
            cout << "6. Exit\nChoice: ";
            cin >> choice;

            try {
                switch (choice) {
                    case 1:
                        ReservationManager::getInstance().viewCustomerReservations(username);
                        break;
                    case 2: {
                        auto strategy = make_shared<AutoAssignTable>();
                        string phoneNumber, date, time;
                        int partySize;
                        cout << "Enter your phone number: ";
                        cin >> phoneNumber;
                        cout << "Enter party size: ";
                        cin >> partySize;
                        cout << "Enter reservation date (e.g., YYYY-MM-DD): ";
                        cin >> date;
                        cout << "Enter reservation time (e.g., HH:MM): ";
                        cin >> time;
                        int table = ReservationManager::getInstance().reserveTable(strategy, username, phoneNumber, partySize, date, time);
                        cout << "Reserved Table #" << table + 1 << " successfully!\n";
                        break;
                    }
                    case 3:
                        ReservationManager::getInstance().viewTableAvailability();
                        break;
                    case 4: {
                        int oldTableIndex;
                        cout << "Enter current table number to update: ";
                        cin >> oldTableIndex;
                        auto strategy = make_shared<AutoAssignTable>();
                        ReservationManager::getInstance().updateReservation(oldTableIndex - 1, username, strategy);
                        cout << "Reservation updated successfully.\n";
                        break;
                    }
                    case 5: {
                        int tableIndex;
                        cout << "Enter table number to cancel: ";
                        cin >> tableIndex;
                        ReservationManager::getInstance().cancelReservation(tableIndex - 1, username);
                        cout << "Reservation cancelled.\n";
                        break;
                    }
                    case 6:
                        isRunning = false;
                        break;
                    default:
                        cout << "Invalid choice.\n";
                }
            } catch (const ReservationException& ex) {
                cout << "Error: " << ex.what() << endl;
            }
        }
    }
};

class Receptionist : public User {
public:
    Receptionist(const string& name) : User(name) {}
    void showMenu() override {
        bool isRunning = true;
        while (isRunning) {
            int choice;
            cout << "\n[Receptionist Menu - " << username << "]\n";
            cout << "1. View Reservations\n2. View Table Availability\n3. Exit\nChoice: ";
            cin >> choice;

            switch (choice) {
                case 1:
                    ReservationManager::getInstance().viewLogs();
                    break;
                case 2:
                    ReservationManager::getInstance().viewTableAvailability();
                    break;
                case 3:
                    isRunning = false;
                    break;
                default:
                    cout << "Invalid choice.\n";
            }
        }
    }
};

class Admin : public User {
public:
    Admin(const string& name) : User(name) {}
    void showMenu() override {
        bool isRunning = true;
        while (isRunning) {
            int choice;
            cout << "\n[Admin Menu - " << username << "]\n";
            cout << "1. View Logs\n2. View Table Availability\n3. Create Receptionist Account\n4. Exit\nChoice: ";
            cin >> choice;

            switch (choice) {
                case 1:
                    ReservationManager::getInstance().viewLogs();
                    break;
                case 2:
                    ReservationManager::getInstance().viewTableAvailability();
                    break;
                case 3: {
                    string recUsername, recPassword;
                    cout << "Enter new receptionist username: ";
                    cin >> recUsername;
                    cout << "Enter password: ";
                    cin >> recPassword;
                    receptionistAccounts[recUsername] = recPassword;
                    cout << "Receptionist account created.\n";
                    break;
                }
                case 4:
                    isRunning = false;
                    break;
                default:
                    cout << "Invalid choice.\n";
            }
        }
    }
};

// -------- Main Driver --------
int main() {
    const string adminUsername = "admin";
    const string adminPassword = "admin123";

    bool isRunning = true;
    while (isRunning) {
        int roleChoice;
        cout << "\n[Role Selection]\n1. Admin\n2. Receptionist\n3. Customer\n4. Exit\nChoose role: ";
        cin >> roleChoice;

        string username, password;
        unique_ptr<User> user;

        switch (roleChoice) {
            case 1:
                cout << "Enter Admin username: ";
                cin >> username;
                cout << "Enter Admin password: ";
                cin >> password;
                if (username == adminUsername && password == adminPassword) {
                    user = make_unique<Admin>(username);
                } else {
                    cout << "Invalid admin credentials.\n";
                    continue;
                }
                break;

            case 2:
                cout << "Enter Receptionist username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                if (receptionistAccounts.count(username) && receptionistAccounts[username] == password) {
                    user = make_unique<Receptionist>(username);
                } else {
                    cout << "Invalid receptionist credentials.\n";
                    continue;
                }
                break;

            case 3: {
                int custOption;
                cout << "\n1. Create Customer Account\n2. Login to Customer Account\nChoice: ";
                cin >> custOption;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                if (custOption == 1) {
                    if (customerAccounts.count(username)) {
                        cout << "Account already exists.\n";
                        continue;
                    }
                    customerAccounts[username] = password;
                    cout << "Customer account created.\n";
                    user = make_unique<Customer>(username);
                } else if (custOption == 2) {
                    if (customerAccounts.count(username) && customerAccounts[username] == password) {
                        user = make_unique<Customer>(username);
                    } else {
                        cout << "Invalid credentials.\n";
                        continue;
                    }
                } else {
                    cout << "Invalid option.\n";
                    continue;
                }
                break;
            }

            case 4:
                isRunning = false;
                continue;

            default:
                cout << "Invalid choice.\n";
                continue;
        }

        user->showMenu();
    }

    return 0;
}