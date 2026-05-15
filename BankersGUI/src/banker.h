#pragma once
#include <vector>
#include <string>
#include <random>

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

// ─── Demo Scenario Data Structure ─────────────────────────────────────────────
struct DemoScenario {
    std::string              name;        // short human-readable name
    int                      processes;
    int                      resources;
    std::vector<std::vector<int>> allocation;
    std::vector<std::vector<int>> maximum;
    std::vector<int>         available;
    bool                     expectedSafe;
    std::string              description; // shown in log
};

struct StepInfo {
    int  processId;
    std::vector<int> workBefore;
    std::vector<int> workAfter;
    bool canExecute;
    std::string message;
};

struct RequestResult {
    bool granted;
    bool wouldWait;          // resources not currently available
    std::string reason;
    std::vector<int> safeSeq;
};

class BankersAlgorithm {
public:
    BankersAlgorithm();

    // Configuration
    void setProcessCount(int n);
    void setResourceCount(int r);
    int  processCount() const { return n_procs; }
    int  resourceCount() const { return n_res; }

    // Matrix accessors
    void setAllocation(int i, int j, int val);
    void setMaximum(int i, int j, int val);
    void setAvailable(int j, int val);

    int  getAllocation(int i, int j) const;
    int  getMaximum(int i, int j) const;
    int  getNeed(int i, int j) const;
    int  getAvailable(int j) const;

    // Derived
    void computeNeed();
    bool validateState(std::string &error) const;

    // Algorithm
    bool runSafetyAlgorithm(std::vector<int> &safeSeq,
                            std::vector<StepInfo> &steps) const;

    RequestResult processRequest(int pid,
                                 const std::vector<int> &request);

    // Snapshot
    void saveSnapshot();
    void restoreSnapshot();
    bool hasSnapshot() const { return snapshotValid; }

    // Demo data
    void loadDemo();                          // legacy single demo
    int  loadRandomDemo();                    // returns index of selected scenario
    const std::vector<DemoScenario>& demoScenarios() const { return m_demos; }
    void reset();

    // Serialisation
    bool saveToFile(const std::string &path) const;
    bool loadFromFile(const std::string &path, std::string &err);

private:
    void initDemos();                         // populate m_demos
    std::vector<DemoScenario> m_demos;
    std::mt19937 m_rng{std::random_device{}()};

    int n_procs = 0;
    int n_res   = 0;

    int allocation [MAX_PROCESSES][MAX_RESOURCES]{};
    int maximum    [MAX_PROCESSES][MAX_RESOURCES]{};
    int need       [MAX_PROCESSES][MAX_RESOURCES]{};
    int available  [MAX_RESOURCES]{};

    // Snapshot
    bool snapshotValid = false;
    int  snap_allocation[MAX_PROCESSES][MAX_RESOURCES]{};
    int  snap_maximum   [MAX_PROCESSES][MAX_RESOURCES]{};
    int  snap_need      [MAX_PROCESSES][MAX_RESOURCES]{};
    int  snap_available [MAX_RESOURCES]{};
};
