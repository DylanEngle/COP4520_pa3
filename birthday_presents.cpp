#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>

using namespace std;

// Present structure
struct Present {
    int tag;
    // Other present attributes if needed
};

// Node structure for linked list
struct Node {
    Present present;
    Node* next;
    Node(const Present& p) : present(p), next(nullptr) {}
};

// Concurrent linked list
class ConcurrentLinkedList {
private:
    Node* head;
    mutex mtx;

public:
    ConcurrentLinkedList() : head(nullptr) {}

    // Add present to the list in ordered manner
    void addPresent(const Present& present) {
        Node* newNode = new Node(present);
        lock_guard<mutex> lock(mtx);
        if (!head || head->present.tag > present.tag) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* current = head;
            while (current->next && current->next->present.tag < present.tag) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
    }

    // Remove present from the list
    void removePresent(int tag) {
        lock_guard<mutex> lock(mtx);
        Node* current = head;
        Node* prev = nullptr;
        while (current) {
            if (current->present.tag == tag) {
                if (prev)
                    prev->next = current->next;
                else
                    head = current->next;
                delete current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    // Search present in the list
    bool searchPresent(int tag) {
        lock_guard<mutex> lock(mtx);
        Node* current = head;
        while (current) {
            if (current->present.tag == tag)
                return true;
            current = current->next;
        }
        return false;
    }
};

// Servant class representing a thread
class Servant {
private:
    string name;
    ConcurrentLinkedList& linkedList;
    queue<Present>& presents;
    mutex& thankYouNotesMtx;
    vector<pair<string, int>>& thankYouNotes;

public:
    Servant(const string& n, ConcurrentLinkedList& ll, queue<Present>& p, mutex& m, vector<pair<string, int>>& tn)
        : name(n), linkedList(ll), presents(p), thankYouNotesMtx(m), thankYouNotes(tn) {}

    void operator()() {
    while (true) {
        Present present;
        {
            lock_guard<mutex> lock(thankYouNotesMtx);
            if (!presents.empty()) {
                present = presents.front();
                presents.pop();
            } else {
                break;
            }
        }
        linkedList.addPresent(present);
        writeThankYouNoteAndRemove(present);
    }
}



    void writeThankYouNoteAndRemove(const Present& present) {
        // Simulate writing thank you note
        {
            lock_guard<mutex> lock(thankYouNotesMtx);
            thankYouNotes.emplace_back(make_pair(name, present.tag));
        }
        linkedList.removePresent(present.tag);
    }
};

int main() {
    const int numServants = 4;
    const int numPresents = 500000;

    queue<Present> presents;
    for (int i = 0; i < numPresents; ++i) {
        presents.push({i}); // Assuming presents have unique tags
    }

    ConcurrentLinkedList linkedList;
    mutex presentsMtx;
    vector<pair<string, int>> thankYouNotes;
    mutex thankYouNotesMtx;

    vector<thread> servants;
    for (int i = 0; i < numServants; ++i) {
        servants.emplace_back(Servant("Servant " + to_string(i + 1), linkedList, presents, thankYouNotesMtx, thankYouNotes));
    }

    for (auto& servant : servants) {
        servant.join();
    }

    if (thankYouNotes.size() < numPresents) {
        cout << "Oops! More presents than thank you notes." << endl;
    } 
    else if(thankYouNotes.size() == numPresents){
        cout << "All presents have been properly acknowledged with thank you notes." << endl;
    }
    else{
        cout << "error";
    }

    return 0;
}
