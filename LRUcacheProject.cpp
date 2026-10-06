#include <iostream>
#include <unordered_map>
using namespace std;

// Node of Doubly Linked List
class Node {
public:
    int key;
    int value;
    Node* prev;
    Node* next;

    Node(int k, int v) {
        key = k;
        value = v;
        prev = NULL;
        next = NULL;
    }
};

class LRUCache {
private:
    int capacity;

    // HashMap:
    // key -> address of corresponding node
    unordered_map<int, Node*> mp;

    // Dummy head and tail
    Node* head;
    Node* tail;

    // Remove a node from the linked list
    void removeNode(Node* node) {
        Node* previous = node->prev;
        Node* nextNode = node->next;

        previous->next = nextNode;
        nextNode->prev = previous;
    }

    // Insert node immediately after head
    void insertAtFront(Node* node) {
        Node* firstNode = head->next;

        node->next = firstNode;
        node->prev = head;

        head->next = node;
        firstNode->prev = node;
    }

public:

    // Constructor
    LRUCache(int capacity) {
        this->capacity = capacity;

        // Dummy nodes
        head = new Node(-1, -1);
        tail = new Node(-1, -1);

        head->next = tail;
        tail->prev = head;
    }

    // Get value corresponding to key
    int get(int key) {

        // Key does not exist
        if (mp.find(key) == mp.end()) {
            return -1;
        }

        // Get node
        Node* node = mp[key];

        // This node was recently used,
        // so move it to the front
        removeNode(node);
        insertAtFront(node);

        return node->value;
    }

    // Insert or update key-value pair
    void put(int key, int value) {

        // Key already exists
        if (mp.find(key) != mp.end()) {

            Node* node = mp[key];

            // Update value
            node->value = value;

            // Move node to front
            removeNode(node);
            insertAtFront(node);

            return;
        }

        // Create new node
        Node* newNode = new Node(key, value);

        // Add to HashMap
        mp[key] = newNode;

        // Add to front of linked list
        insertAtFront(newNode);

        // Capacity exceeded
        if (mp.size() > capacity) {

            // Least recently used node
            Node* lruNode = tail->prev;

            // Remove from linked list
            removeNode(lruNode);

            // Remove from HashMap
            mp.erase(lruNode->key);

            // Free memory
            delete lruNode;
        }
    }

    // Display cache from most recently used to least recently used
    void display() {

        Node* current = head->next;

        cout << "Cache: ";

        while (current != tail) {
            cout << "(" << current->key
                 << "," << current->value << ") ";

            current = current->next;
        }

        cout << endl;
    }

    // Destructor
    ~LRUCache() {

        Node* current = head;

        while (current != NULL) {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }
};

int main() {

    // Cache capacity = 3
    LRUCache cache(3);

    cout << "Adding elements:\n";

    cache.put(1, 10);
    cache.display();

    cache.put(2, 20);
    cache.display();

    cache.put(3, 30);
    cache.display();

    cout << "\nAccessing key 1:\n";

    cout << "Value = " << cache.get(1) << endl;
    cache.display();

    cout << "\nAdding key 4:\n";

    cache.put(4, 40);
    cache.display();

    cout << "\nTrying to access key 2:\n";

    cout << "Value = " << cache.get(2) << endl;
    cache.display();

    cout << "\nUpdating key 3:\n";

    cache.put(3, 300);
    cache.display();

    cout << "\nAccessing key 4:\n";

    cout << "Value = " << cache.get(4) << endl;
    cache.display();

    return 0;
}