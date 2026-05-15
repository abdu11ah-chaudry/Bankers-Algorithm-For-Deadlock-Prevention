#include "banker.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

BankersAlgorithm::BankersAlgorithm() {
    reset();
    initDemos();
}

// ─── Demo Scenario Definitions ────────────────────────────────────────────────────
void BankersAlgorithm::initDemos() {
    m_demos.clear();

    // ===== SAFE STATE SCENARIOS =====

    // SAFE #1  — Classic Silberschatz textbook (5P, 3R)
    {
        DemoScenario d;
        d.name        = "Silberschatz Classic (5P/3R)";
        d.processes   = 5;
        d.resources   = 3;
        d.allocation  = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
        d.maximum     = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
        d.available   = {3,3,2};
        d.expectedSafe = true;
        d.description  = "Silberschatz textbook 5-process 3-resource example. "
                         "Safe sequence: P1→P3→P4→P0→P2";
        m_demos.push_back(d);
    }

    // SAFE #2  — Small 3-process, 2-resource system
    {
        DemoScenario d;
        d.name        = "Small System (3P/2R)";
        d.processes   = 3;
        d.resources   = 2;
        d.allocation  = {{1,0},{0,1},{1,1}};
        d.maximum     = {{2,2},{1,2},{2,2}};
        d.available   = {1,1};
        d.expectedSafe = true;
        d.description  = "Minimal 3-process 2-resource safe scenario. "
                         "Ideal for introductory demonstrations.";
        m_demos.push_back(d);
    }

    // SAFE #3  — 4-process, 4-resource heterogeneous system
    {
        DemoScenario d;
        d.name        = "Heterogeneous (4P/4R)";
        d.processes   = 4;
        d.resources   = 4;
        d.allocation  = {{0,0,1,2},{1,0,0,0},{1,3,5,4},{0,6,3,2}};
        d.maximum     = {{0,0,1,2},{1,7,5,0},{2,3,5,6},{0,6,5,2}};
        d.available   = {1,5,2,0};
        d.expectedSafe = true;
        d.description  = "4-process 4-resource safe state. "
                         "Demonstrates heterogeneous resource types.";
        m_demos.push_back(d);
    }

    // SAFE #4  — 6-process, 3-resource medium-scale
    {
        DemoScenario d;
        d.name        = "Medium Scale (6P/3R)";
        d.processes   = 6;
        d.resources   = 3;
        d.allocation  = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2},{1,0,1}};
        d.maximum     = {{4,2,1},{3,2,1},{5,1,3},{3,2,2},{2,2,3},{2,1,2}};
        d.available   = {2,2,1};
        d.expectedSafe = true;
        d.description  = "6-process medium-scale safe scenario. "
                         "Good for demonstrating multi-pass safety checks.";
        m_demos.push_back(d);
    }

    // SAFE #5  — High available resources — trivially safe
    {
        DemoScenario d;
        d.name        = "Resource-Rich (4P/3R)";
        d.processes   = 4;
        d.resources   = 3;
        d.allocation  = {{1,0,1},{0,1,0},{1,1,0},{0,0,1}};
        d.maximum     = {{3,2,2},{2,3,1},{3,2,1},{1,2,2}};
        d.available   = {5,4,3};
        d.expectedSafe = true;
        d.description  = "Abundant available resources make any sequence safe. "
                         "Illustrates how high availability trivially satisfies all needs.";
        m_demos.push_back(d);
    }

    // ===== UNSAFE STATE SCENARIOS =====

    // UNSAFE #1  — Classic circular deadlock (4P, 3R)
    {
        DemoScenario d;
        d.name        = "Circular Deadlock (4P/3R)";
        d.processes   = 4;
        d.resources   = 3;
        d.allocation  = {{1,0,0},{0,1,0},{0,0,1},{1,1,1}};
        d.maximum     = {{2,1,1},{1,2,1},{1,1,2},{2,2,2}};
        d.available   = {0,0,0};
        d.expectedSafe = false;
        d.description  = "No available resources and circular dependencies. "
                         "Classic deadlock — no safe sequence exists.";
        m_demos.push_back(d);
    }

    // UNSAFE #2  — Over-allocated single resource type
    {
        DemoScenario d;
        d.name        = "Over-Allocated R0 (3P/2R)";
        d.processes   = 3;
        d.resources   = 2;
        d.allocation  = {{2,1},{3,0},{2,1}};
        d.maximum     = {{4,2},{5,2},{3,2}};
        d.available   = {0,0};
        d.expectedSafe = false;
        d.description  = "All resource instances are allocated and every process "
                         "still needs more. Impossible to make progress.";
        m_demos.push_back(d);
    }

    // UNSAFE #3  — Near-safe but one process blocks all
    {
        DemoScenario d;
        d.name        = "One Blocker (5P/3R)";
        d.processes   = 5;
        d.resources   = 3;
        d.allocation  = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
        d.maximum     = {{7,5,3},{3,2,2},{9,3,2},{2,2,2},{4,3,3}};
        d.available   = {1,0,0};
        d.expectedSafe = false;
        d.description  = "Based on the Silberschatz example but P2's maximum and "
                         "available resources are tweaked so no safe ordering exists.";
        m_demos.push_back(d);
    }

    // UNSAFE #4  — 4-process, 4-resource all-held scenario
    {
        DemoScenario d;
        d.name        = "All-Held Deadlock (4P/4R)";
        d.processes   = 4;
        d.resources   = 4;
        d.allocation  = {{1,1,0,0},{0,0,1,0},{0,0,0,1},{1,0,0,1}};
        d.maximum     = {{2,2,1,0},{1,1,2,1},{1,0,1,2},{2,1,0,2}};
        d.available   = {0,0,0,0};
        d.expectedSafe = false;
        d.description  = "All resource instances held with zero available. "
                         "4-resource deadlock with mutual hold-and-wait.";
        m_demos.push_back(d);
    }

    // UNSAFE #5  — Nearly sufficient resources, one starved process
    {
        DemoScenario d;
        d.name        = "Starved Process (5P/2R)";
        d.processes   = 5;
        d.resources   = 2;
        d.allocation  = {{1,1},{1,0},{0,1},{1,0},{0,1}};
        d.maximum     = {{2,2},{3,2},{1,3},{2,2},{1,3}};
        d.available   = {1,0};
        d.expectedSafe = false;
        d.description  = "One resource type fully exhausted. Process P1 and P2 "
                         "cannot finish, blocking the rest in a chain.";
        m_demos.push_back(d);
    }
}

// ─── Random Demo Loader ───────────────────────────────────────────────────────────
int BankersAlgorithm::loadRandomDemo() {
    if (m_demos.empty()) initDemos();
    std::uniform_int_distribution<int> dist(0, static_cast<int>(m_demos.size()) - 1);
    int idx = dist(m_rng);
    const DemoScenario &d = m_demos[idx];

    n_procs = d.processes;
    n_res   = d.resources;
    memset(allocation, 0, sizeof(allocation));
    memset(maximum,    0, sizeof(maximum));
    memset(available,  0, sizeof(available));

    for (int i = 0; i < n_procs; ++i)
        for (int j = 0; j < n_res; ++j) {
            allocation[i][j] = d.allocation[i][j];
            maximum[i][j]    = d.maximum[i][j];
        }
    for (int j = 0; j < n_res; ++j)
        available[j] = d.available[j];

    computeNeed();
    snapshotValid = false;
    return idx;
}

void BankersAlgorithm::reset() {
    n_procs = 0; n_res = 0;
    memset(allocation, 0, sizeof(allocation));
    memset(maximum,    0, sizeof(maximum));
    memset(need,       0, sizeof(need));
    memset(available,  0, sizeof(available));
    snapshotValid = false;
}

void BankersAlgorithm::setProcessCount(int n)  { n_procs = n; }
void BankersAlgorithm::setResourceCount(int r)  { n_res   = r; }

void BankersAlgorithm::setAllocation(int i,int j,int val){ allocation[i][j]=val; }
void BankersAlgorithm::setMaximum   (int i,int j,int val){ maximum[i][j]   =val; }
void BankersAlgorithm::setAvailable (int j,    int val)  { available[j]    =val; }

int BankersAlgorithm::getAllocation(int i,int j)const{ return allocation[i][j]; }
int BankersAlgorithm::getMaximum   (int i,int j)const{ return maximum[i][j];    }
int BankersAlgorithm::getNeed      (int i,int j)const{ return need[i][j];       }
int BankersAlgorithm::getAvailable (int j)      const{ return available[j];     }

void BankersAlgorithm::computeNeed() {
    for(int i=0;i<n_procs;i++)
        for(int j=0;j<n_res;j++)
            need[i][j] = maximum[i][j] - allocation[i][j];
}

bool BankersAlgorithm::validateState(std::string &error) const {
    for(int i=0;i<n_procs;i++)
        for(int j=0;j<n_res;j++){
            if(allocation[i][j] < 0){
                error = "Allocation["+std::to_string(i)+"]["+std::to_string(j)+"] is negative.";
                return false;
            }
            if(maximum[i][j] < allocation[i][j]){
                error = "Maximum["+std::to_string(i)+"]["+std::to_string(j)+
                        "] < Allocation. Maximum must be >= Allocation.";
                return false;
            }
        }
    for(int j=0;j<n_res;j++)
        if(available[j]<0){
            error = "Available["+std::to_string(j)+"] is negative.";
            return false;
        }
    return true;
}

bool BankersAlgorithm::runSafetyAlgorithm(std::vector<int> &safeSeq,
                                           std::vector<StepInfo> &steps) const
{
    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES]{};
    memcpy(work, available, n_res * sizeof(int));
    safeSeq.clear();
    steps.clear();

    int count = 0;
    while(count < n_procs){
        bool found = false;
        for(int i=0;i<n_procs;i++){
            if(finish[i]) continue;
            bool ok = true;
            for(int j=0;j<n_res;j++)
                if(need[i][j] > work[j]){ ok=false; break; }

            StepInfo si;
            si.processId  = i;
            si.workBefore = std::vector<int>(work, work+n_res);
            si.canExecute = ok;

            if(ok){
                std::string msg = "P"+std::to_string(i)+" can execute: Need[";
                for(int j=0;j<n_res;j++) msg+=(j?",":"")+std::to_string(need[i][j]);
                msg += "] ≤ Work[";
                for(int j=0;j<n_res;j++) msg+=(j?",":"")+std::to_string(work[j]);
                msg += "]. Resources released.";
                si.message = msg;

                for(int j=0;j<n_res;j++) work[j] += allocation[i][j];
                si.workAfter = std::vector<int>(work, work+n_res);
                finish[i] = true;
                safeSeq.push_back(i);
                found = true;
                steps.push_back(si);
                count++;
            } else {
                std::string msg = "P"+std::to_string(i)+" cannot execute: Need[";
                for(int j=0;j<n_res;j++) msg+=(j?",":"")+std::to_string(need[i][j]);
                msg += "] > Work[";
                for(int j=0;j<n_res;j++) msg+=(j?",":"")+std::to_string(work[j]);
                msg += "]. Must wait.";
                si.message   = msg;
                si.workAfter = std::vector<int>(work, work+n_res);
                steps.push_back(si);
            }
        }
        if(!found) break;
    }
    return (count == n_procs);
}

RequestResult BankersAlgorithm::processRequest(int pid,
                                                const std::vector<int> &request)
{
    RequestResult res;
    res.granted   = false;
    res.wouldWait = false;

    // Step 1: request <= need?
    for(int j=0;j<n_res;j++){
        if(request[j] > need[pid][j]){
            res.reason = "DENIED: Request["+std::to_string(j)+
                         "] = "+std::to_string(request[j])+
                         " exceeds declared maximum need ("+
                         std::to_string(need[pid][j])+").";
            return res;
        }
    }
    // Step 2: request <= available?
    for(int j=0;j<n_res;j++){
        if(request[j] > available[j]){
            res.wouldWait = true;
            res.reason = "WAIT: Resources not currently available. P"+
                         std::to_string(pid)+" must wait.";
            return res;
        }
    }
    // Step 3: tentative allocation
    for(int j=0;j<n_res;j++){
        available[j]       -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j]       -= request[j];
    }
    // Step 4: safety check
    std::vector<StepInfo> steps;
    bool safe = runSafetyAlgorithm(res.safeSeq, steps);

    if(safe){
        res.granted = true;
        res.reason  = "GRANTED: System remains in safe state after allocation.";
    } else {
        // rollback
        for(int j=0;j<n_res;j++){
            available[j]       += request[j];
            allocation[pid][j] -= request[j];
            need[pid][j]       += request[j];
        }
        res.reason = "DENIED: Granting this request would lead to an UNSAFE state. Rolled back.";
    }
    return res;
}

void BankersAlgorithm::saveSnapshot() {
    memcpy(snap_allocation, allocation, sizeof(allocation));
    memcpy(snap_maximum,    maximum,    sizeof(maximum));
    memcpy(snap_need,       need,       sizeof(need));
    memcpy(snap_available,  available,  sizeof(available));
    snapshotValid = true;
}

void BankersAlgorithm::restoreSnapshot() {
    if(!snapshotValid) return;
    memcpy(allocation, snap_allocation, sizeof(allocation));
    memcpy(maximum,    snap_maximum,    sizeof(maximum));
    memcpy(need,       snap_need,       sizeof(need));
    memcpy(available,  snap_available,  sizeof(available));
}

void BankersAlgorithm::loadDemo() {
    // Legacy: loads scenario #0 (Silberschatz classic)
    if (m_demos.empty()) initDemos();
    const DemoScenario &d = m_demos[0];
    n_procs = d.processes; n_res = d.resources;
    memset(allocation, 0, sizeof(allocation));
    memset(maximum,    0, sizeof(maximum));
    memset(available,  0, sizeof(available));
    for (int i = 0; i < n_procs; ++i)
        for (int j = 0; j < n_res; ++j) {
            allocation[i][j] = d.allocation[i][j];
            maximum[i][j]    = d.maximum[i][j];
        }
    for (int j = 0; j < n_res; ++j) available[j] = d.available[j];
    computeNeed();
    snapshotValid = false;
}

bool BankersAlgorithm::saveToFile(const std::string &path) const {
    std::ofstream f(path);
    if(!f) return false;
    f << n_procs << " " << n_res << "\n";
    for(int j=0;j<n_res;j++) f << available[j] << (j+1<n_res?" ":"\n");
    for(int i=0;i<n_procs;i++){
        for(int j=0;j<n_res;j++) f << allocation[i][j] << (j+1<n_res?" ":"\n");
    }
    for(int i=0;i<n_procs;i++){
        for(int j=0;j<n_res;j++) f << maximum[i][j] << (j+1<n_res?" ":"\n");
    }
    return true;
}

bool BankersAlgorithm::loadFromFile(const std::string &path, std::string &err) {
    std::ifstream f(path);
    if(!f){ err="Cannot open file."; return false; }
    int np,nr;
    if(!(f>>np>>nr)){ err="Bad header."; return false; }
    if(np<1||np>MAX_PROCESSES||nr<1||nr>MAX_RESOURCES){
        err="Process/resource count out of range."; return false;
    }
    n_procs=np; n_res=nr;
    for(int j=0;j<n_res;j++) if(!(f>>available[j])){ err="Bad available."; return false; }
    for(int i=0;i<n_procs;i++)
        for(int j=0;j<n_res;j++) if(!(f>>allocation[i][j])){ err="Bad allocation."; return false; }
    for(int i=0;i<n_procs;i++)
        for(int j=0;j<n_res;j++) if(!(f>>maximum[i][j])){ err="Bad maximum."; return false; }
    computeNeed();
    return true;
}
