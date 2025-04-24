#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <limits>
#include <memory>

using namespace std;

// Enhanced DateTime class with validation
class DateTime {
private:
    tm time_struct;
    bool valid = false;

    void validate() {
        time_t temp = mktime(&time_struct);
        valid = temp != -1;
    }

public:
    DateTime() {}
    
    DateTime(const string& datetime_str) {
        istringstream ss(datetime_str);
        ss >> get_time(&time_struct, "%Y-%m-%d %H:%M");
        validate();
    }

    DateTime(time_t timestamp) {
        localtime_r(&timestamp, &time_struct);
        valid = true;
    }

    bool isValid() const { return valid; }
    bool isFuture() const { return valid && to_time_t() > time(nullptr); }

    time_t to_time_t() const {
        tm tm_copy = time_struct;
        return mktime(&tm_copy);
    }

    bool operator<(const DateTime& other) const { return to_time_t() < other.to_time_t(); }
    bool operator>(const DateTime& other) const { return to_time_t() > other.to_time_t(); }
    bool operator<=(const DateTime& other) const { return to_time_t() <= other.to_time_t(); }
    bool operator>=(const DateTime& other) const { return to_time_t() >= other.to_time_t(); }
    bool operator==(const DateTime& other) const { return to_time_t() == other.to_time_t(); }

    friend ostream& operator<<(ostream& os, const DateTime& dt) {
        tm tm_copy = dt.time_struct;
        os << put_time(&tm_copy, "%Y-%m-%d %H:%M");
        return os;
    }
};

class User {
public:
    int id;
    string username;
    string password;
    
    User(int id, string uname, string pwd)
        : id(id), username(move(uname)), password(move(pwd)) {}
};

class UserManager {
private:
    unordered_map<string, shared_ptr<User>> users;
    int next_id = 1;

public:
    bool registerUser(const string& username, const string& password) {
        if (users.count(username)) return false;
        users[username] = make_shared<User>(next_id++, username, password);
        return true;
    }

    shared_ptr<User> authenticate(const string& username, const string& password) {
        auto it = users.find(username);
        if (it != users.end() && it->second->password == password) {
            return it->second;
        }
        return nullptr;
    }

    bool usernameExists(const string& username) const {
        return users.count(username);
    }
};

class Event {
public:
    int event_id;
    string title;
    DateTime date_time;
    bool is_public;
    int creator_id;
    string description;
    set<int> attendees;

    Event(int id, string t, DateTime dt, bool pub, int creator, string desc = "")
        : event_id(id), title(t), date_time(dt), is_public(pub),
          creator_id(creator), description(desc) {}
};

class BSTNode {
public:
    DateTime key;
    vector<Event*> events;
    BSTNode* left;
    BSTNode* right;

    BSTNode(DateTime dt, Event* event) 
        : key(dt), left(nullptr), right(nullptr) {
        events.push_back(event);
    }
};

class EventBST {
private:
    BSTNode* root;

    BSTNode* insertRec(BSTNode* node, Event* event) {
        if (!node) return new BSTNode(event->date_time, event);
        
        if (event->date_time < node->key) {
            node->left = insertRec(node->left, event);
        } else if (event->date_time > node->key) {
            node->right = insertRec(node->right, event);
        } else {
            node->events.push_back(event);
        }
        return node;
    }

    void rangeSearchRec(BSTNode* node, const DateTime& start, const DateTime& end,
                       vector<Event*>& results, int current_user) {
        if (!node) return;

        if (node->key >= start && node->key <= end) {
            for (Event* event : node->events) {
                if (checkAccess(event, current_user)) {
                    results.push_back(event);
                }
            }
            rangeSearchRec(node->left, start, end, results, current_user);
            rangeSearchRec(node->right, start, end, results, current_user);
        }
        else if (node->key < start) {
            rangeSearchRec(node->right, start, end, results, current_user);
        }
        else {
            rangeSearchRec(node->left, start, end, results, current_user);
        }
    } 

    bool checkAccess(Event* event, int current_user) {
        return event->is_public || 
               (current_user != -1 && (event->creator_id == current_user ||
                event->attendees.count(current_user)));
    }

public:
    EventBST() : root(nullptr) {}

    void insert(Event* event) {
        root = insertRec(root, event);
    }

    vector<Event*> rangeSearch(const DateTime& start, const DateTime& end, int current_user) {
        vector<Event*> results;
        rangeSearchRec(root, start, end, results, current_user);
        return results;
    }
};

class EventManager {
private:
    EventBST bst;
    unordered_map<int, Event*> events;
    shared_ptr<User> current_user;
    UserManager user_manager;
    int next_event_id = 1;

    bool checkAccess(Event* event) {
        return event->is_public || 
               (current_user && (event->creator_id == current_user->id ||
                event->attendees.count(current_user->id)));
    }

public:
    bool isLoggedIn() const { return current_user != nullptr; }
    string currentUsername() const { return current_user ? current_user->username : ""; }

    void createEvent(const string& title, const string& date_str, bool is_public, const string& desc) {
        DateTime dt(date_str);
        if (!dt.isValid() || !dt.isFuture()) return;
        
        Event* event = new Event(next_event_id++, title, dt, is_public, 
                                current_user->id, desc);
        bst.insert(event);
        events[event->event_id] = event;
    }

    void registerUsers() {
        user_manager.registerUser("admin", "admin");
        current_user = user_manager.authenticate("admin", "admin");
    }
    
    bool registerUser() {
        string username, password;
        cout << "Enter username: ";
        getline(cin, username);
        if (user_manager.usernameExists(username)) {
            cout << "✗ Username already exists.\n";
            return false;
        }
        cout << "Enter password: ";
        getline(cin, password);
        return user_manager.registerUser(username, password);
    }

    bool login() {
        string username, password;
        cout << "Enter username: ";
        getline(cin, username);
        cout << "Enter password: ";
        getline(cin, password);
        
        current_user = user_manager.authenticate(username, password);
        return current_user != nullptr;
    }

    void logout() {
        current_user.reset();
    }

    void createEvent() {
        if (!current_user) return;

        string title, date_str, desc;
        bool is_public;
        DateTime dt;

        cout << "Enter event title: ";
        getline(cin, title);
        
        while (true) {
            cout << "Enter event date (YYYY-MM-DD HH:MM): ";
            getline(cin, date_str);
            dt = DateTime(date_str);
            
            if (!dt.isValid()) {
                cout << "✗ Invalid date format. Please try again.\n";
                continue;
            }
            if (!dt.isFuture()) {
                cout << "✗ Event date must be in the future.\n";
                continue;
            }
            break;
        }

        cout << "Is public? (1 for yes, 0 for no): ";
        cin >> is_public;
        cin.ignore();

        cout << "Enter description: ";
        getline(cin, desc);

        try {
            Event* event = new Event(next_event_id++, title, dt, is_public, 
                                    current_user->id, desc);
            bst.insert(event);
            events[event->event_id] = event;
            cout << "✓ Event created successfully! (ID: " << event->event_id << ")\n";
        } catch (const exception& e) {
            cout << "✗ Error creating event: " << e.what() << "\n";
        }
    }

    vector<Event*> getUpcomingEvents(int days = 7) {
        time_t now = time(nullptr);
        DateTime start(now);
        DateTime end(now + days * 24 * 3600);
        return bst.rangeSearch(start, end, current_user ? current_user->id : -1);
    }

    bool rsvp(int event_id) {
        if (!current_user) return false;
        auto it = events.find(event_id);
        if (it != events.end() && checkAccess(it->second)) {
            it->second->attendees.insert(current_user->id);
            return true;
        }
        return false;
    }

    vector<Event*> searchEvents(const string& query) {
        vector<Event*> results;
        string lower_query;
        transform(query.begin(), query.end(), back_inserter(lower_query), ::tolower);

        for (auto& pair : events) {
            Event* event = pair.second;
            if (checkAccess(event)) {
                string title = event->title;
                transform(title.begin(), title.end(), title.begin(), ::tolower);
                
                string desc = event->description;
                transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

                if (title.find(lower_query) != string::npos ||
                    desc.find(lower_query) != string::npos) {
                    results.push_back(event);
                }
            }
        }
        return results;
    }

    vector<Event*> getUserEvents() {
        vector<Event*> user_events;
        if (!current_user) return user_events;

        for (auto& pair : events) {
            Event* event = pair.second;
            if (event->creator_id == current_user->id || 
                event->attendees.count(current_user->id)) {
                user_events.push_back(event);
            }
        }
        sort(user_events.begin(), user_events.end(),
            [](Event* a, Event* b) { return a->date_time < b->date_time; });
        return user_events;
    }
};

// UI Components
void displayEventDetails(const Event* event) {
    cout << "\n┌──────────────────────────────────┐\n";
    cout << "│ " << setw(30) << left << event->title << "   │\n";
    cout << "├──────────────────────────────────┤\n";
    cout << "│ ID: " << setw(26) << left << event->event_id << "   │\n";
    cout << "│ Time: " << setw(24) << left << event->date_time << "           │\n";
    cout << " Type: " << (event->is_public ? "Public" : "Private")<< " \n";
    cout << "│ Creator: "<<setw(5) << event->creator_id << "                   │\n";
    cout << "│ Attendees: " << setw(19) << left << event->attendees.size() << "   │\n";
    if (!event->description.empty()) {
        cout << "├──────────────────────────────────┤\n";
        cout << "│ " << setw(30) << left << (event->description.length() > 30 ? event->description.substr(0, 27) + "..." : event->description) << "   │\n";
    }
    cout << "└──────────────────────────────────┘\n";
}

void displayEventList(const vector<Event*>& events, const string& title) {
    if (events.empty()) {
        cout << "\nNo " << title << " found.\n";
        return;
    }

    cout << "\n┌──────┬──────────────────────┬───────────────────┬─────────┬──────────┐\n";
    cout << "│ " << setw(4) << left << "ID" << " │ " << setw(20) << left << "Title" 
         << " │ " << setw(17) << left << "Date/Time" << " │ " << setw(7) << left << "Type" 
         << " │ " << setw(8) << left << "Attendees" << "│\n";
    cout << "├──────┼──────────────────────┼───────────────────┼─────────┼──────────┤\n";

    for (const auto& event : events) {
        cout << "│ " << setw(4) << left << event->event_id 
             << " │ " << setw(20) << left << (event->title.length() > 20 ? event->title.substr(0, 17) + "..." : event->title)
             << " │ " << left<< event->date_time 
             << "  │ " << setw(7) << left << (event->is_public ? "Public" : "Private")
             << " │ " << setw(8) << left << event->attendees.size() << " │\n";
    }
    cout << "└──────┴──────────────────────┴───────────────────┴─────────┴──────────┘\n";
}

void displayMenu(bool isLoggedIn, const string& username = "") {
    cout << "\n╔══════════════════════════════════╗\n";
    cout << "║        EVENT ORGANIZER           ║\n";
    cout << "╠══════════════════════════════════╣\n";
    if (isLoggedIn) {
        cout << "║ Logged in as: " << setw(18) << left << username << " ║\n";
    }
    cout << "╠──────────────────────────────────╣\n";
    cout << "║ 1. " << setw(6)<<(isLoggedIn ? "Logout" : "Login") << setw(24) << left << "" << "║\n";
    if (!isLoggedIn) {
        cout << "║ 2. Register" << setw(20) << left << "" << "  ║\n";
    } else {
        cout << "║ 2. Create New Event" << setw(13) << left << "" << " ║\n";
        cout << "║ 3. RSVP to Event" << setw(15) << left << "" << "  ║\n";
        cout << "║ 4. My Events" << setw(18) << left << "" << "   ║\n";
    }
    cout << "║ " << (isLoggedIn ? "5" : "3") << ". Search Events" << setw(16) << left << "" << " ║\n";
    cout << "║ " << (isLoggedIn ? "6" : "4") << ". Upcoming Events" << setw(14) << left << "" << " ║\n";
    cout << "║ " << (isLoggedIn ? "7" : "5") << ". Exit" << setw(25) << left << "" << " ║\n";
    cout << "╚══════════════════════════════════╝\n";
    cout << "Enter your choice: ";
}

void handleEventSearch(EventManager& manager) {
    string query;
    cout << "\nEnter search query (or leave empty for all events): ";
    getline(cin, query);
    
    auto results = manager.searchEvents(query);
    displayEventList(results, "events matching '" + query + "'");
    
    if (!results.empty()) {
        cout << "Enter event ID to view details (0 to cancel): ";
        int id;
        if (cin >> id && id != 0) {
            auto it = find_if(results.begin(), results.end(), 
                [id](Event* e) { return e->event_id == id; });
            if (it != results.end()) {
                displayEventDetails(*it);
            } else {
                cout << "✗ Event not found.\n";
            }
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void displayUpcomingEvents(EventManager& manager) {
    int days;
    cout << "\nShow events for how many days? (default 7): ";
    string input;
    getline(cin, input);
    
    try {
        days = input.empty() ? 7 : stoi(input);
    } catch (...) {
        days = 7;
    }
    
    auto upcoming = manager.getUpcomingEvents(days);
    displayEventList(upcoming, "upcoming events");
}

void handleUserEvents(EventManager& manager) {
    auto userEvents = manager.getUserEvents();
    displayEventList(userEvents, "your events");
    
    if (!userEvents.empty()) {
        cout << "\nOptions:\n";
        cout << "1. View event details\n";
        cout << "0. Back to main menu\n";
        cout << "Enter choice: ";
        
        int choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (choice == 1) {
            cout << "Enter event ID: ";
            int id;
            cin >> id;
            auto it = find_if(userEvents.begin(), userEvents.end(),
                [id](Event* e) { return e->event_id == id; });
            if (it != userEvents.end()) {
                displayEventDetails(*it);
            } else {
                cout << "✗ Event not found in your events.\n";
            }
        }
    }
}

int main() {
    EventManager manager;
    {
        manager.registerUsers();
        //manager.login();
        vector<tuple<string, string, bool, string>> sampleEvents = {
            {"Tech Conference", "2025-10-15 09:00", true, "Annual technology summit featuring AI and blockchain talks"},
            {"Birthday Party", "2025-07-20 19:30", false, "Private celebration with friends"},
            {"Marathon", "2025-09-01 07:00", true, "City marathon open to all residents"},
            {"Book Club", "2025-08-05 18:00", false, "Discussion of 'The Midnight Library'"},
            {"Art Exhibition", "2025-11-10 10:00", true, "Modern art showcase with local artists"}
        };

        for(const auto& [title, date, isPublic, desc] : sampleEvents) {
            manager.createEvent(title, date, isPublic, desc);
        }
        
        manager.logout();
    }

    int choice;
    bool running = true;

    while (running) {
        bool isLoggedIn = manager.isLoggedIn();
        string username = manager.currentUsername();
        displayMenu(isLoggedIn, username);
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "✗ Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: 
                if (isLoggedIn) {
                    manager.logout();
                    cout << "✓ Logged out successfully.\n";
                } else {
                    if (manager.login()) {
                        cout << "✓ Login successful!\n";
                    } else {
                        cout << "✗ Invalid credentials.\n";
                    }
                }
                break;

            case 2: 
                if (!isLoggedIn) {
                    if (manager.registerUser()) {
                        cout << "✓ Registration successful!\n";
                    } else {
                        cout << "✗ Registration failed.\n";
                    }
                } else {
                    manager.createEvent();
                }
                break;

            case 3: // RSVP or Search
                if (isLoggedIn) {
                    int event_id;
                    cout << "Enter event ID to RSVP: ";
                    cin >> event_id;
                    cin.ignore();
                    if (manager.rsvp(event_id)) {
                        cout << "✓ RSVP successful!\n";
                    } else {
                        cout << "✗ RSVP failed. Invalid event ID or access denied.\n";
                    }
                } else {
                    handleEventSearch(manager);
                }
                break;

            case 4: // My Events or Upcoming
                if (isLoggedIn) {
                    handleUserEvents(manager);
                } else {
                    displayUpcomingEvents(manager);
                }
                break;

            case 5: // Search or Exit
                if (isLoggedIn) {
                    handleEventSearch(manager);
                } else {
                    running = false;
                    cout << "Goodbye!\n";
                }
                break;

            case 6: if (isLoggedIn) displayUpcomingEvents(manager); break;
            case 7: if (isLoggedIn) running = false; break;

            default:
                cout << "✗ Invalid choice. Please try again.\n";
                break;
        }
    }

    cout << "Exiting...\n";
    return 0;
}