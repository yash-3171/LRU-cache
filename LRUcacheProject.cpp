#include <iostream>
#include <unordered_map>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;


// ============================================================
// NODE
// ============================================================

class Node {
public:

    int key;
    int value;

    // Time after which this entry expires
    long long expiryTime;

    Node* prev;
    Node* next;

    Node(int k, int v, long long expiry) {

        key = k;
        value = v;
        expiryTime = expiry;

        prev = NULL;
        next = NULL;
    }
};


// ============================================================
// LRU CACHE
// ============================================================

class LRUCache {

private:

    int capacity;

    // HashMap:
    // key -> address of Node
    unordered_map<int, Node*> cache;

    // Dummy nodes
    Node* head;
    Node* tail;

    // Statistics
    long long hits;
    long long misses;


    // --------------------------------------------------------
    // Get current time in milliseconds
    // --------------------------------------------------------

    long long getCurrentTime() {

        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }


    // --------------------------------------------------------
    // Check whether a node has expired
    // --------------------------------------------------------

    bool isExpired(Node* node) {

        // expiryTime = -1 means no expiration
        if (node->expiryTime == -1) {
            return false;
        }

        return getCurrentTime() >= node->expiryTime;
    }


    // --------------------------------------------------------
    // Remove node from doubly linked list
    // --------------------------------------------------------

    void removeNode(Node* node) {

        Node* previous = node->prev;
        Node* nextNode = node->next;

        previous->next = nextNode;
        nextNode->prev = previous;
    }


    // --------------------------------------------------------
    // Insert node immediately after HEAD
    // --------------------------------------------------------

    void insertAtFront(Node* node) {

        Node* firstNode = head->next;

        node->next = firstNode;
        node->prev = head;

        head->next = node;
        firstNode->prev = node;
    }


    // --------------------------------------------------------
    // Remove expired nodes
    // --------------------------------------------------------

    void removeExpiredNodes() {

        Node* current = head->next;

        while (current != tail) {

            Node* nextNode = current->next;

            if (isExpired(current)) {

                cache.erase(current->key);

                removeNode(current);

                delete current;
            }

            current = nextNode;
        }
    }


public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    LRUCache(int capacity) {

        this->capacity = capacity;

        hits = 0;
        misses = 0;

        // Dummy head and tail
        head = new Node(-1, -1, -1);
        tail = new Node(-1, -1, -1);

        head->next = tail;
        tail->prev = head;
    }


    // ========================================================
    // GET
    // ========================================================

    int get(int key) {

        // Key does not exist
        if (cache.find(key) == cache.end()) {

            misses++;

            return -1;
        }


        Node* node = cache[key];


        // Check expiration
        if (isExpired(node)) {

            cache.erase(key);

            removeNode(node);

            delete node;

            misses++;

            return -1;
        }


        // Key exists and is valid
        hits++;


        // Move node to front
        removeNode(node);

        insertAtFront(node);


        return node->value;
    }


    // ========================================================
    // PUT
    // ========================================================

    void put(int key, int value, int ttlSeconds = -1) {

        // Calculate expiration time
        long long expiryTime = -1;

        if (ttlSeconds > 0) {

            expiryTime =
                getCurrentTime()
                +
                (long long)ttlSeconds * 1000;
        }


        // ----------------------------------------------------
        // KEY ALREADY EXISTS
        // ----------------------------------------------------

        if (cache.find(key) != cache.end()) {

            Node* node = cache[key];


            // Update value
            node->value = value;

            // Update TTL
            node->expiryTime = expiryTime;


            // Move to front
            removeNode(node);

            insertAtFront(node);

            return;
        }


        // ----------------------------------------------------
        // CREATE NEW NODE
        // ----------------------------------------------------

        Node* newNode =
            new Node(key, value, expiryTime);


        // Add to HashMap
        cache[key] = newNode;


        // Add to front
        insertAtFront(newNode);


        // ----------------------------------------------------
        // CAPACITY EXCEEDED
        // ----------------------------------------------------

        if ((int)cache.size() > capacity) {

            // Least Recently Used node
            Node* lruNode = tail->prev;


            // Remove from list
            removeNode(lruNode);


            // Remove from HashMap
            cache.erase(lruNode->key);


            // Free memory
            delete lruNode;
        }
    }


    // ========================================================
    // REMOVE
    // ========================================================

    void remove(int key) {

        if (cache.find(key) == cache.end()) {

            cout << "Key not found.\n";

            return;
        }


        Node* node = cache[key];


        removeNode(node);

        cache.erase(key);

        delete node;


        cout << "Key removed successfully.\n";
    }


    // ========================================================
    // CLEAR
    // ========================================================

    void clear() {

        Node* current = head->next;


        while (current != tail) {

            Node* nextNode = current->next;

            delete current;

            current = nextNode;
        }


        head->next = tail;
        tail->prev = head;


        cache.clear();


        cout << "Cache cleared successfully.\n";
    }


    // ========================================================
    // DISPLAY CACHE
    // ========================================================

    void display() {

        removeExpiredNodes();


        if (cache.empty()) {

            cout << "\nCache is empty.\n";

            return;
        }


        cout << "\n";
        cout << "----------------------------------------\n";
        cout << "Cache (MRU -> LRU)\n";
        cout << "----------------------------------------\n";


        Node* current = head->next;


        while (current != tail) {

            cout << "[Key: "
                 << current->key
                 << ", Value: "
                 << current->value;


            if (current->expiryTime == -1) {

                cout << ", TTL: None";

            } else {

                long long remaining =
                    current->expiryTime
                    - getCurrentTime();

                if (remaining < 0) {
                    remaining = 0;
                }

                cout << ", TTL: "
                     << remaining / 1000
                     << "s";
            }


            cout << "]";


            if (current->next != tail) {

                cout << " -> ";
            }


            current = current->next;
        }


        cout << "\n";
        cout << "----------------------------------------\n";
    }


    // ========================================================
    // SHOW STATISTICS
    // ========================================================

    void showStatistics() {

        long long totalRequests =
            hits + misses;


        cout << "\n";
        cout << "========================================\n";
        cout << "           CACHE STATISTICS\n";
        cout << "========================================\n";


        cout << "Capacity       : "
             << capacity << endl;


        cout << "Current Size   : "
             << cache.size() << endl;


        cout << "Cache Hits     : "
             << hits << endl;


        cout << "Cache Misses   : "
             << misses << endl;


        cout << "Total Requests : "
             << totalRequests << endl;


        if (totalRequests > 0) {

            double hitRatio =
                ((double)hits / totalRequests) * 100;


            double missRatio =
                ((double)misses / totalRequests) * 100;


            cout << fixed << setprecision(2);


            cout << "Hit Ratio      : "
                 << hitRatio
                 << "%" << endl;


            cout << "Miss Ratio     : "
                 << missRatio
                 << "%" << endl;
        }
        else {

            cout << "Hit Ratio      : 0%\n";
            cout << "Miss Ratio     : 0%\n";
        }


        cout << "========================================\n";
    }


    // ========================================================
    // SIZE
    // ========================================================

    int size() {

        removeExpiredNodes();

        return cache.size();
    }


    // ========================================================
    // CHECK EMPTY
    // ========================================================

    bool isEmpty() {

        removeExpiredNodes();

        return cache.empty();
    }


    // ========================================================
    // RESET STATISTICS
    // ========================================================

    void resetStatistics() {

        hits = 0;
        misses = 0;

        cout << "Statistics reset successfully.\n";
    }


    // ========================================================
    // DESTRUCTOR
    // ========================================================

    ~LRUCache() {

        Node* current = head;


        while (current != NULL) {

            Node* nextNode = current->next;

            delete current;

            current = nextNode;
        }
    }
};


// ============================================================
// MAIN
// ============================================================

int main() {

    int capacity;


    cout << "========================================\n";
    cout << "          LRU CACHE SIMULATOR\n";
    cout << "========================================\n";


    cout << "Enter cache capacity: ";
    cin >> capacity;


    if (capacity <= 0) {

        cout << "Capacity must be greater than 0.\n";

        return 0;
    }


    LRUCache cache(capacity);


    int choice;


    while (true) {

        cout << "\n";
        cout << "========================================\n";
        cout << "              MENU\n";
        cout << "========================================\n";

        cout << "1. PUT\n";
        cout << "2. GET\n";
        cout << "3. REMOVE\n";
        cout << "4. DISPLAY CACHE\n";
        cout << "5. SHOW STATISTICS\n";
        cout << "6. CLEAR CACHE\n";
        cout << "7. RESET STATISTICS\n";
        cout << "8. SHOW CACHE SIZE\n";
        cout << "9. EXIT\n";

        cout << "========================================\n";


        cout << "Enter your choice: ";
        cin >> choice;


        // ====================================================
        // PUT
        // ====================================================

        if (choice == 1) {

            int key;
            int value;
            int ttl;


            cout << "Enter key: ";
            cin >> key;


            cout << "Enter value: ";
            cin >> value;


            cout << "Enter TTL in seconds (-1 for no expiration): ";
            cin >> ttl;


            cache.put(key, value, ttl);


            cout << "\nData inserted successfully.\n";
        }


        // ====================================================
        // GET
        // ====================================================

        else if (choice == 2) {

            int key;


            cout << "Enter key: ";
            cin >> key;


            int value = cache.get(key);


            if (value == -1) {

                cout << "\nCache MISS.\n";
                cout << "Key does not exist or has expired.\n";
            }

            else {

                cout << "\nCache HIT.\n";
                cout << "Value = "
                     << value
                     << endl;
            }
        }


        // ====================================================
        // REMOVE
        // ====================================================

        else if (choice == 3) {

            int key;


            cout << "Enter key: ";
            cin >> key;


            cache.remove(key);
        }


        // ====================================================
        // DISPLAY
        // ====================================================

        else if (choice == 4) {

            cache.display();
        }


        // ====================================================
        // STATISTICS
        // ====================================================

        else if (choice == 5) {

            cache.showStatistics();
        }


        // ====================================================
        // CLEAR
        // ====================================================

        else if (choice == 6) {

            cache.clear();
        }


        // ====================================================
        // RESET STATISTICS
        // ====================================================

        else if (choice == 7) {

            cache.resetStatistics();
        }


        // ====================================================
        // SIZE
        // ====================================================

        else if (choice == 8) {

            cout << "\nCurrent cache size: "
                 << cache.size()
                 << endl;
        }


        // ====================================================
        // EXIT
        // ====================================================

        else if (choice == 9) {

            cout << "\nExiting LRU Cache Simulator...\n";

            break;
        }


        // ====================================================
        // INVALID CHOICE
        // ====================================================

        else {

            cout << "\nInvalid choice. Please try again.\n";
        }
    }


    return 0;
}
