#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <random>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iomanip>
#include <sstream>

using namespace std;

// Constants
const int NUM_SENSORS = 8;
const int READINGS_PER_HOUR = 60;
const int REPORT_INTERVAL = 1; // simulation time
const int NUM_REPORTS = 5; // Number of reports to generate

// Data structure for temperature reading
struct Reading {
    int temperature; // Fahrenheit
    chrono::time_point<chrono::system_clock> timestamp;
};

// Shared circular buffer for storing temperature readings
class CircularBuffer {
private:
    vector<Reading> buffer;
    int capacity;
    int size;
    int head;
    int tail;
    mutex mtx;
    condition_variable not_full;
    condition_variable not_empty;

public:
    CircularBuffer(int capacity) : capacity(capacity), size(0), head(0), tail(0), buffer(capacity) {}

    void write(const Reading& reading) {
        unique_lock<mutex> lock(mtx);
        not_full.wait(lock, [this]() { return size != capacity; });
        buffer[head] = reading;
        head = (head + 1) % capacity;
        size++;
        not_empty.notify_all();
    }

    vector<Reading> readAll() {
        unique_lock<mutex> lock(mtx);
        not_empty.wait(lock, [this]() { return size != 0; });
        vector<Reading> readings;
        int current = tail;
        for (int i = 0; i < size; ++i) {
            readings.push_back(buffer[current]);
            current = (current + 1) % capacity;
        }
        size = 0;
        tail = head; // Reset tail to head
        not_full.notify_all();
        return readings;
    }
};

// Function to generate random temperature readings
int generateRandomTemperature() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 70);
    return dis(gen);
}

// Sensor thread function
void sensorThread(int id, CircularBuffer& buffer) {
    while (true) {
        Reading reading;
        reading.temperature = generateRandomTemperature();
        reading.timestamp = chrono::system_clock::now();
        buffer.write(reading);
        this_thread::sleep_for(chrono::milliseconds(100)); // Read temperature every period
    }
}

// Function to compile report
void compileReport(CircularBuffer& buffer) {
    for (int reportCount = 0; reportCount < NUM_REPORTS; ++reportCount) {
        cout << "Compiling report " << reportCount + 1 << "...." << endl;
        this_thread::sleep_for(chrono::seconds(REPORT_INTERVAL)); // Compile report every "hour"
        vector<Reading> readings = buffer.readAll();

        // Sort readings by temperature
        sort(readings.begin(), readings.end(), [](const Reading& a, const Reading& b) {
            return a.temperature < b.temperature;
        });

        // Top 5 highest temperatures
        cout << "Top 5 highest temperatures of the hour:" << endl;
        for (int i = readings.size() - 1; i >= max(0, (int)readings.size() - 5); --i) {
            cout << "Temperature: " << readings[i].temperature << "F, Timestamp: ";
            time_t time = chrono::system_clock::to_time_t(readings[i].timestamp);
            cout << ctime(&time);
        }

        // Top 5 lowest temperatures
        cout << "Top 5 lowest temperatures of the hour:" << endl;
        for (int i = 0; i < min(5, (int)readings.size()); ++i) {
            cout << "Temperature: " << readings[i].temperature << "F, Timestamp: ";
            time_t time = chrono::system_clock::to_time_t(readings[i].timestamp);
            cout << ctime(&time);
        }

        // Find the 10-minute interval with the largest temperature difference (1 second for simulation purposes)
        int maxDifference = 0;
        chrono::time_point<chrono::system_clock> maxDiffStart, maxDiffEnd;
        if(readings.size() > 10){

            for (int i = 0; i < readings.size() - 10; ++i) {
                int difference = readings[i + 10].temperature - readings[i].temperature;
                if (difference > maxDifference) {
                    maxDifference = difference;
                    maxDiffStart = readings[i].timestamp;
                    maxDiffEnd = readings[i + 10].timestamp;
                }
            }

            // Convert time_point to tm struct for formatting
            time_t startT = chrono::system_clock::to_time_t(maxDiffStart);
            tm startTm = *localtime(&startT);

            time_t endT = chrono::system_clock::to_time_t(maxDiffEnd);
            tm endTm = *localtime(&endT);

            // Format and display the start and end times
            stringstream startSs, endSs;
            startSs << put_time(&startTm, "%Y-%m-%d %H:%M:%S");
            endSs << put_time(&endTm, "%Y-%m-%d %H:%M:%S");

            cout << "Largest temperature difference observed over a 10-minute interval:" << endl;
            cout << "Start Time: " << startSs.str() << endl;
            cout << "End Time: " << endSs.str() << endl;
        }
    }
}

int main() {
    // Initialize circular buffer
    CircularBuffer buffer(READINGS_PER_HOUR * NUM_REPORTS * NUM_SENSORS); // Buffer size for 5 hours of readings

    // Create sensor threads
    vector<thread> sensorThreads;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        sensorThreads.emplace_back(sensorThread, i + 1, ref(buffer));
    }

    // Create reporting thread
    thread reportingThread(compileReport, ref(buffer));

    // Join reporting thread
    reportingThread.join();

    // Terminate sensor threads
    for (auto& thread : sensorThreads) {
        if (thread.joinable()) {
            thread.detach();
        }
    }

    return 0;
}
