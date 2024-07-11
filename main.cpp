#include <iostream>
#include <sqlite3.h>
#include <string>
#include <iomanip>
using namespace std;

// Clase para manejar la base de datos
class Database {
public:
    Database(const string &db_name);
    ~Database();
    bool execute(const string &query);
    sqlite3* getDB() const { return db; }

private:
    sqlite3 *db;
};

Database::Database(const string &db_name) {
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::execute(const string &query) {
    char *errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Funciones para realizar operaciones CRUD
void createTables(Database &db) {
    string createUsersTable = "CREATE TABLE IF NOT EXISTS Users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "name TEXT NOT NULL, "
                              "email TEXT NOT NULL UNIQUE);";
    
    string createGamesTable = "CREATE TABLE IF NOT EXISTS Games ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "title TEXT NOT NULL UNIQUE, "
                              "genre TEXT NOT NULL);";
    
    string createUsersGamesTable = "CREATE TABLE IF NOT EXISTS Users_Games ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                   "user_id INTEGER, "
                                   "game_id INTEGER, "
                                   "purchase_date TEXT, "
                                   "price REAL, "
                                   "FOREIGN KEY(user_id) REFERENCES Users(id), "
                                   "FOREIGN KEY(game_id) REFERENCES Games(id));";

    if (db.execute(createUsersTable) &&
        db.execute(createGamesTable) &&
        db.execute(createUsersGamesTable)) {
        cout << "Tables created successfully." << endl;
    } else {
        cout << "Error creating tables." << endl;
    }
}

void insertUser(Database &db, const string &name, const string &email) {
    string query = "INSERT INTO Users (name, email) VALUES ('" + name + "', '" + email + "');";
    if (db.execute(query)) {
        cout << "User added successfully." << endl;
    } else {
        cout << "Error adding user." << endl;
    }
}

void insertGame(Database &db, const string &title, const string &genre) {
    string query = "INSERT INTO Games (title, genre) VALUES ('" + title + "', '" + genre + "');";
    if (db.execute(query)) {
        cout << "Game added successfully." << endl;
    } else {
        cout << "Error adding game." << endl;
    }
}

void associateUserGame(Database &db, int user_id, int game_id) {
    string query = "INSERT INTO Users_Games (user_id, game_id) "
                   "VALUES (" + to_string(user_id) + ", " + to_string(game_id) + ");";
    if (db.execute(query)) {
        cout << "User associated with game successfully." << endl;
    } else {
        cout << "Error associating user with game." << endl;
    }
}

void listUsers(Database &db) {
    string query = "SELECT * FROM Users;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        cout << "Users:\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            const unsigned char *email = sqlite3_column_text(stmt, 2);
            cout << "ID: " << id << ", Name: " << name << ", Email: " << email << endl;
        }
        cout << endl;
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing users." << endl;
    }
}

void listGames(Database &db) {
    string query = "SELECT * FROM Games;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        cout << "Games:\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *title = sqlite3_column_text(stmt, 1);
            const unsigned char *genre = sqlite3_column_text(stmt, 2);
            cout << "ID: " << id << ", Title: " << title << ", Genre: " << genre << endl;
        }
        cout << endl;
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing games." << endl;
    }
}

void listUserGames(Database &db, int user_id) {
    string query = "SELECT Users.name, Games.title, Users_Games.purchase_date, Users_Games.price "
                   "FROM Users "
                   "JOIN Users_Games ON Users.id = Users_Games.user_id "
                   "JOIN Games ON Users_Games.game_id = Games.id "
                   "WHERE Users.id = " + to_string(user_id) + ";";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        cout << "Games associated with User ID " << user_id << ":\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *username = sqlite3_column_text(stmt, 0);
            const unsigned char *gametitle = sqlite3_column_text(stmt, 1);
            const unsigned char *purchase_date = sqlite3_column_text(stmt, 2);
            double price = sqlite3_column_double(stmt, 3);
            
            cout << "User: " << username << ", Game: " << gametitle << ", Purchase Date: " << purchase_date
                 << ", Price: $" << fixed << setprecision(2) << price << endl;
        }
        cout << endl;
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing user's games." << endl;
    }
}

void listGameUsers(Database &db, int game_id) {
    string query = "SELECT Users.name, Games.title, Users_Games.purchase_date, Users_Games.price "
                   "FROM Users "
                   "JOIN Users_Games ON Users.id = Users_Games.user_id "
                   "JOIN Games ON Users_Games.game_id = Games.id "
                   "WHERE Games.id = " + to_string(game_id) + ";";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        cout << "Users who have purchased Game ID " << game_id << ":\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *username = sqlite3_column_text(stmt, 0);
            const unsigned char *gametitle = sqlite3_column_text(stmt, 1);
            const unsigned char *purchase_date = sqlite3_column_text(stmt, 2);
            double price = sqlite3_column_double(stmt, 3);
            
            cout << "User: " << username << ", Game: " << gametitle << ", Purchase Date: " << purchase_date
                 << ", Price: $" << fixed << setprecision(2) << price << endl;
        }
        cout << endl;
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing game's users." << endl;
    }
}

int main() {
    Database db("user_game.db");

    createTables(db);

    int option;
    do {
        cout << "Menu:\n";
        cout << "1. Add User\n";
        cout << "2. Add Game\n";
        cout << "3. Associate User with Game \n";
        cout << "4. List Users\n";
        cout << "5. List Games\n";
        cout << "6. List Games of a User\n";
        cout << "7. List Users of a Game\n";
        cout << "0. Exit\n";
        cout << "Choose an option: ";
        cin >> option;
        cin.ignore();

        switch (option) {
            case 1: {
                string name, email;
                cout << "Enter name: ";
                getline(cin, name);
                cout << "Enter email: ";
                getline(cin, email);
                insertUser(db, name, email);
                break;
            }
            case 2: {
                string title, genre;
                cout << "Enter title: ";
                getline(cin, title);
                cout << "Enter genre: ";
                getline(cin, genre);
                insertGame(db, title, genre);
                break;
            }
            case 3: {
                int user_id, game_id;
                string purchase_date;
                float price;
                cout << "Enter User ID: ";
                cin >> user_id;
                cout << "Enter Game ID: ";
                cin >> game_id;
                cin.ignore();
                associateUserGame(db, user_id, game_id);
                break;
            }
            case 4:
                listUsers(db);
                break;
            case 5:
                listGames(db);
                break;
            case 6: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                listUserGames(db, user_id);
                break;
            }
            case 7: {
                int game_id;
                cout << "Enter Game ID: ";
                cin >> game_id;
                listGameUsers(db, game_id);
                break;
            }
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid option. Please try again.\n";
        }
    } while (option != 0);

    return 0;
}
