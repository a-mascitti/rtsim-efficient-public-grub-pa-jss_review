#include <metasim.hpp>
#include <rttask.hpp>
#include <mrtkernel.hpp>
#include <energyMRTKernel.hpp>
#include <edfsched.hpp>
#include <cbserver.hpp>
#include <taskstat.hpp>
#include <fileImporter.hpp>

#include "catch.hpp"

using namespace MetaSim;
using namespace RTSim;
using namespace std;

#define time()              		SIMUL.getTime()
#define FORCE_REQUISITES    		1 /* 1 if you want to try all experiments */
#define TEST_NO             		init_sequence
#define LONG_EXPERIMENT_ENABLED		false
#define LONG_EXPERIMENT_RAND_PER	false

string workload = "bzip2";
string task_name = "";
int init_sequence = 0;

class Requisite {
    public:
        bool    satisfied;
        bool    EMRTK_leave_little3, EMRTK_migrate, EMRTK_cbs_yield, EMRTK_temporarily_migrate_vtime, EMRTK_temporarily_migrate_end,
                EMRTK_cbs_enveloping_per_task_enabled, EMRTK_cbs_enveloping_migrate_after_vtime_end, EMRTK_cbs_migrate_after_end
                ;

        // in the default case, you don't want neither leave_little3 and migrate and CBS servers yielding
        Requisite(bool EMRTK_leave_little3 = false, bool EMRTK_migrate = true, bool EMRTK_cbs_yield = false,
            bool EMRTK_cbs_enveloping_per_task_enabled = true, bool EMRTK_cbs_enveloping_migrate_after_vtime_end = false, bool EMRTK_cbs_migrate_after_end = true,
            bool EMRTK_temporarily_migrate_vtime = false, bool EMRTK_temporarily_migrate_end = false)
            : satisfied(false) {
                this->EMRTK_leave_little3                           = EMRTK_leave_little3;
                this->EMRTK_migrate                                 = EMRTK_migrate;
                this->EMRTK_cbs_yield                               = EMRTK_cbs_yield;
                this->EMRTK_cbs_enveloping_per_task_enabled         = EMRTK_cbs_enveloping_per_task_enabled;
                this->EMRTK_cbs_enveloping_migrate_after_vtime_end  = EMRTK_cbs_enveloping_migrate_after_vtime_end;
                this->EMRTK_cbs_migrate_after_end                   = EMRTK_cbs_migrate_after_end;
                this->EMRTK_temporarily_migrate_vtime               = EMRTK_temporarily_migrate_vtime;
                this->EMRTK_temporarily_migrate_end                 = EMRTK_temporarily_migrate_end;
            }

        string toString() const {
            string s = "Requisite with leave_little_3: " + to_string(EMRTK_leave_little3) + ", migrate: " + to_string(EMRTK_migrate) + ", cbs yielding: " + to_string(EMRTK_cbs_yield) + 
                ", cbs_enveloping_per_task_enabled: " + to_string(EMRTK_cbs_enveloping_per_task_enabled) + ", cbs_enveloping_migrate_after_vtime_end: " + to_string(EMRTK_cbs_enveloping_migrate_after_vtime_end) + 
                ", cbs_migrate_after_end: " + to_string(EMRTK_cbs_migrate_after_end) +
                ", EMRTK_temporarily_migrate_vtime: " + to_string(EMRTK_temporarily_migrate_vtime) + ", EMRTK_temporarily_migrate_end: " + to_string(EMRTK_temporarily_migrate_end);
            return s;
        }
};

map<int, Requisite> performedTests;

void getCores(vector<CPU_BL*> &big, vector<CPU_BL*> &little, Island_BL **island_bl_little, Island_BL **island_bl_big);
int  init_suite(EnergyMRTKernel** kern);
bool isInRange(int,int);
bool isInRange(Tick t1, int t2) { return isInRange(int(t1), t2); }
bool isInRange(Tick t1, Tick t2) { return isInRange(int(t1), int(t2)); }
bool isInRangeMinMax(double eval, const double min, const double max);
bool checkRequisites(Requisite reqs);
bool fixDependencies(Requisite reqs, bool abortOnFix = false);

TEST_CASE("exp0") {
    init_sequence = 0;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    task_name = "T0_task1";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t0 = new PeriodicTask(500, 500, 0, task_name);
    t0->insertCode("fixed(500," + workload + ");"); // WCET 500 at max frequency on big cores
    CBServerCallingEMRTKernel* et_t0 = kern->addTaskAndEnvelope(t0, "");

    SIMUL.initSingleRun();
    SIMUL.run_to(1);
    CPU_BL *c0 = dynamic_cast<CPU_BL*>(dynamic_cast<CPU_BL*>(kern->getProcessor(et_t0)));

    REQUIRE (c0->getFrequency() == 2000);
    REQUIRE (c0->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange (int(t0->getWCET(c0->getSpeed())), 497));
    REQUIRE (et_t0->getStatus() == ServerStatus::EXECUTING);
    REQUIRE (et_t0->getPeriod() == 500);
    REQUIRE (isInRange (et_t0->getBudget(), 497));
    REQUIRE (isInRange (et_t0->getEndBandwidthEvent(), 497));
    REQUIRE (isInRange (et_t0->getReplenishmentEvent(), 500));

    SIMUL.run_to(499);
    cout << "end of virtualtime event: t=" << et_t0->getEndOfVirtualTimeEvent() << endl;
    REQUIRE (isInRange (et_t0->getEndOfVirtualTimeEvent(), 500));
    // REQUIRE (et_t0->getStatus() == ServerStatus::RELEASING);
    REQUIRE (et_t0->getStatus() == ServerStatus::RECHARGING);

    SIMUL.run_to(501);
    REQUIRE (c0->getFrequency() == 2000);
    REQUIRE (c0->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t0->getWCET(c0->getSpeed())), 497));
    REQUIRE (et_t0->getStatus() == ServerStatus::EXECUTING);
    REQUIRE (et_t0->getPeriod() == 500);
    REQUIRE (isInRange (et_t0->getBudget(), 497));
    REQUIRE (isInRange (et_t0->getEndBandwidthEvent(), 997));
    REQUIRE (isInRange (et_t0->getReplenishmentEvent(), 500));

    SIMUL.run_to(999);
    cout << "end of virtualtime event: t=" << et_t0->getEndOfVirtualTimeEvent() << endl;
    REQUIRE (isInRange (et_t0->getEndOfVirtualTimeEvent(), 1000));

    SIMUL.endSingleRun();
    delete t0;
    delete kern;
    delete et_t0;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp1") {
    init_sequence = 1;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    task_name = "T1_task1";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t0 = new PeriodicTask(500, 500, 0, task_name);
    t0->insertCode("fixed(500," + workload + ");"); // WCET 500 at max frequency on big cores
    CBServerCallingEMRTKernel* et_t0 = kern->addTaskAndEnvelope(t0, "");

    task_name = "T1_task2";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t1 = new PeriodicTask(500, 500, 0, task_name);
    t1->insertCode("fixed(500," + workload + ");");
    CBServerCallingEMRTKernel* et_t1 = kern->addTaskAndEnvelope(t1, "");

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    CPU_BL *c0 = dynamic_cast<CPU_BL*>(kern->getProcessor(et_t0));
    CPU_BL *c1 = dynamic_cast<CPU_BL*>(kern->getProcessor(et_t1));

    REQUIRE (c0->getFrequency() == 2000);
    REQUIRE (c0->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t0->getWCET(c0->getSpeed())), 497));

    REQUIRE (c1->getFrequency() == 2000);
    REQUIRE (c1->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t1->getWCET(c1->getSpeed())), 497));

    SIMUL.run_to(1000);

    SIMUL.endSingleRun();
    delete t1; delete t0;
    delete kern;
    delete et_t0; delete et_t1;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp2") {
    init_sequence = 2;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    task_name = "T2_task1";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t0 = new PeriodicTask(500, 500, 0, task_name);
    t0->insertCode("fixed(500," + workload + ");"); // WCET 500 at max frequency on big cores
    CBServerCallingEMRTKernel* et_t0 = kern->addTaskAndEnvelope(t0, "");

    task_name = "T2_task2";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t1 = new PeriodicTask(500, 500, 0, task_name);
    t1->insertCode("fixed(250," + workload + ");");
    CBServerCallingEMRTKernel* et_t1 = kern->addTaskAndEnvelope(t1, "");

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    CPU_BL *c0 = dynamic_cast<CPU_BL*>(kern->getProcessor(et_t0));
    CPU_BL *c1 = dynamic_cast<CPU_BL*>(kern->getProcessor(et_t1));

    for(string s : kern->getRunningTasks())
      cout << "running :" << s<<endl;

    REQUIRE (t0->getName() == "T2_task1");
    REQUIRE (c0->getFrequency() == 2000);
    REQUIRE (c0->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t0->getWCET(c0->getSpeed())), 497));

    REQUIRE (t1->getName() == "T2_task2");
    REQUIRE (c1->getFrequency() == 2000);
    REQUIRE (c1->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t1->getWCET(c1->getSpeed())), 248));

    SIMUL.run_to(1000);
    SIMUL.endSingleRun();
    delete t0; delete t1;
    delete kern;
    delete et_t0; delete et_t1;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp3") {
    init_sequence = 3;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    task_name = "T3_task1";
    cout << "Creating task: " << task_name << endl;
    PeriodicTask* t0 = new PeriodicTask(500, 500, 0, task_name);
    t0->insertCode("fixed(10," + workload + ");"); // WCET 10 at max frequency on big cores
    CBServerCallingEMRTKernel* et_t0 = kern->addTaskAndEnvelope(t0, "");

    SIMUL.initSingleRun();
    SIMUL.run_to(10);

    cout << "t=" << SIMUL.getTime() << endl;
    CPU_BL *c0 = dynamic_cast<CPU_BL*>(kern->getProcessor(et_t0));

    REQUIRE (t0->getName() == "T3_task1");
    REQUIRE (c0->getFrequency() == 200);
    REQUIRE (c0->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t0->getWCET(c0->getSpeed())), 155));

    SIMUL.endSingleRun();
    delete t0;
    delete kern;
    delete et_t0;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp4") {
    init_sequence = 4;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    PeriodicTask* task[5]; // to be cleared after each test
    vector<CBServerCallingEMRTKernel*> ets;
    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    for (int j = 0; j < 4; j++) {
        task_name = "T4_Task_LITTLE_" + std::to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(500, 500, 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(100, %s);", workload.c_str());
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));

        task[j] = t;
    }

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    CPU_BL *c0 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(0)));
    CPU_BL *c1 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(1)));
    CPU_BL *c2 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(2)));
    CPU_BL *c3 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(3)));

    REQUIRE (task[0]->getName() == "T4_Task_LITTLE_0");
    REQUIRE (c0->getFrequency() == 700);
    REQUIRE (c0->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(task[0]->getWCET(c0->getSpeed())), 488));

    REQUIRE (task[1]->getName() == "T4_Task_LITTLE_1");
    REQUIRE (c1->getFrequency() == 700);
    REQUIRE (c1->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(task[1]->getWCET(c1->getSpeed())), 488));

    REQUIRE (task[2]->getName() == "T4_Task_LITTLE_2");
    REQUIRE (c2->getFrequency() == 700);
    REQUIRE (c2->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(task[2]->getWCET(c2->getSpeed())), 488));

    REQUIRE (task[3]->getName() == "T4_Task_LITTLE_3");
    REQUIRE (c3->getFrequency() == 700);
    REQUIRE (c3->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(task[3]->getWCET(c3->getSpeed())), 488));

    SIMUL.endSingleRun();
    for (int j = 0; j < 4; j++) {
        delete task[j];
        delete ets.at(j);
    }
    delete kern;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp5") {
    init_sequence = 5;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    vector<CPU_BL*> cpus;
    PeriodicTask* task[5]; // to be cleared after each test
    vector<CBServerCallingEMRTKernel*> ets;
    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    for (int j = 0; j < 4; j++) {
        int wcet = 5; //* (j+1);
        task_name = "T5_task" + std::to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(500, 500, 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcet, workload.c_str());
        cout << " with abs. WCET " << wcet << endl;
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));

        task[j] = t;
        // LITTLE_0, _1, _2, _3 freq 1400.
    }

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    CPU_BL *c0 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(0)));
    CPU_BL *c1 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(1)));
    CPU_BL *c2 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(2)));
    CPU_BL *c3 = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(3)));
    cout << c0->toString() << endl;
    cout << c1->toString() << endl;
    cout << c2->toString() << endl;
    cout << c3->toString() << endl;

    PeriodicTask* t = task[0];
    REQUIRE (t->getName() == "T5_task0");
    REQUIRE (c0->getFrequency() == 200);
    REQUIRE (c0->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c0->getSpeed())), 78));
    cout << t->toString() << " on "<< c0->toString()<<endl;

    t = task[1];
    REQUIRE (t->getName() == "T5_task1");
    REQUIRE (c1->getFrequency() == 200);
    REQUIRE (c1->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c1->getSpeed())), 78));
    cout << t->toString() << " on "<< c1->toString()<<endl;

    t = task[2];
    REQUIRE (t->getName() == "T5_task2");
    REQUIRE (c2->getFrequency() == 200);
    REQUIRE (c2->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c2->getSpeed())), 78));
    cout << t->toString() << " on "<< c2->toString()<<endl;

    t = task[3];
    REQUIRE (t->getName() == "T5_task3");
    REQUIRE (c3->getFrequency() == 200);
    REQUIRE (c3->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c3->getSpeed())), 78));
    cout << t->toString() << " on "<< c3->toString()<<endl;

    SIMUL.endSingleRun();
    for (int j = 0; j < 4; j++) {
        delete task[j];
        delete ets.at(j);
    }
    delete kern;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

// test showing that frequency of little/big island may be raised
TEST_CASE("exp6") {
    /* This experiment shows that other CPUs frequencies are increased for the
        entire island after a decision for other tasks has already been made*/
    init_sequence = 6;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    vector<CPU_BL*> cpus;
    CPU_BL* cpu_task[5]; // to be cleared after each test
    PeriodicTask* task[5]; // to be cleared after each test
    vector<CBServerCallingEMRTKernel*> ets;
    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    int i, wcet = 300;
    for (int j = 0; j < 5; j++) {
        if (j == 4)
            wcet = 200;
        task_name = "T6_task" + std::to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(500, 500, 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcet, workload.c_str());
        cout << " with abs. WCET " << wcet << endl;
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));

        task[j] = t;
    }

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    for (int j = 0; j < 5; j++) {
    	cout << j << endl;
        cpu_task[j] = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(j)));
    	if (j == 4)
    		cpu_task[j] = kern->getProcessorReady(ets.at(j));
    }

    i = 0;
    PeriodicTask* t = task[i];
    CPU_BL* c = cpu_task[i];
    REQUIRE (t->getName() == "T6_task0");
    REQUIRE (c->getFrequency() == 2000);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 299));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 1;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T6_task1");
    REQUIRE (c->getFrequency() == 2000);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 299));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 2;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T6_task2");
    REQUIRE (c->getFrequency() == 2000);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 299));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 3;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T6_task3");
    REQUIRE (c->getFrequency() == 2000);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())),299));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 4;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T6_task4");
    REQUIRE (c->getFrequency() == 2000);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 199));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    SIMUL.endSingleRun();
    for (int j = 0; j < 5; j++) {
        delete task[j];
        delete ets.at(j);
    }
    delete kern;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp7") {
    init_sequence = 7;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    vector<CPU_BL*> cpus;
    PeriodicTask* task[5]; // to be cleared after each test
    CPU_BL* cpu_task[5]; // to be cleared after each test
    vector<CBServerCallingEMRTKernel*> ets;
    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    int wcets[] = { 63, 63, 63, 63, 30 };
    int i;
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        task_name = "T7_task" + std::to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(500, 500, 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcets[j], workload.c_str());
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));

        task[j] = t;
    }

    /* Towards random workloads, but this time alg. first decides to
       schedule all tasks on littles, and then, instead of schedule the
       next one in bigs, it shall increase littles frequency so to make
       space to it too and save energy */
    SIMUL.initSingleRun();
    for (CPU_BL* c : cpus)
        assert (c->getWorkload() == "idle");
    SIMUL.run_to(1);
    cout << "t=1, first task scheduling decided" << endl;
    kern->printState(true, true);


    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        cpu_task[j] = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(j)));
    }

    i = 0;
    PeriodicTask* t = task[i];
    CPU_BL* c = cpu_task[i];
    REQUIRE (t->getName() == "T7_task0");
    REQUIRE (c->getFrequency() == 500);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 415));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 1;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T7_task1");
    REQUIRE (c->getFrequency() == 500);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 415));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 2;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T7_task2");
    REQUIRE (c->getFrequency() == 500);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 415));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 3;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T7_task3");
    REQUIRE (c->getFrequency() == 500);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 415));
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));

    i = 4;
    t = task[i];
    c = cpu_task[i];
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    kern->printState(1);
    REQUIRE (t->getName() == "T7_task4");
    REQUIRE (c->getFrequency() == 200);//700);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 259));//75));

    SIMUL.run_to(1000);
    SIMUL.endSingleRun();
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        delete task[j];
        delete ets.at(j);
    }
    delete kern;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}

TEST_CASE("exp8") {
    init_sequence = 8;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req;
    if (!checkRequisites( req ))  return;

    vector<CPU_BL*> cpus;
    PeriodicTask* task[9]; // to be cleared after each test
    CPU_BL* cpu_task[9]; // to be cleared after each test
    vector<CBServerCallingEMRTKernel*> ets;
    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);

    int wcets[] = { 181, 419, 261, 163, 65, 8, 61, 170, 273 };
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        task_name = "T8_task" + std::to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(500, 500, 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcets[j], workload.c_str());
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));
        task[j] = t;
    }
    // towards random workloads...

    SIMUL.initSingleRun();
    SIMUL.run_to(1);

    kern->printState(1);

    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        cpu_task[j] = dynamic_cast<CPU_BL*>(kern->getProcessor(ets.at(j)));
    }

    int i = 0;
    PeriodicTask* t = task[i];
    CPU_BL* c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1800);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 196));

    i = 1;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1800);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 454));

    i = 2;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1800);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 297));

    i = 3;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1300);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 476));

    i = 4;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1300);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 189));

    i = 5;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1300);
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 22));

    i = 6;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    cout << "a"<<endl;
    REQUIRE (c->getFrequency() == 1300);
    cout << "b"<<endl;
    REQUIRE (c->getIslandType() == IslandType::LITTLE);
    cout << "c"<<endl;
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 178));

    i = 7;
    t = task[i];
    c = cpu_task[i];
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1800);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 184));

    i = 8;
    t = task[i];
    c = kern->getProcessorReady(t);
    REQUIRE (t->getName() == "T8_task" + to_string(i));
    REQUIRE (c->getFrequency() == 1800);
    REQUIRE (c->getIslandType() == IslandType::BIG);
    printf("aaa %s scheduled on %s freq %lu with wcet %f\n", t->getName().c_str(), c->toString().c_str(), c->getFrequency(), t->getWCET(c->getSpeed()));
    REQUIRE (isInRange(int(t->getWCET(c->getSpeed())), 310));

    SIMUL.endSingleRun();
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        delete task[j];
        delete ets.at(j);
    }
    delete kern;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}


/// Experiments suggested by Mr.Marinoni: ready after task ends, migrate after vtime, dispatch at period begin.
/// These are actually useless tests because if EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END_ADV_CHK is
/// disabled, the algorithm is not correct

TEST_CASE("Experiment 20") {
    /**
        2nd example. End of virtual time of task t. Kernel pulls (migrates) a task into
        the ending core. It has WCET > DL_t (t is now idle). When t arrives again, it can
        be scheduled on its previous core.
      */
    init_sequence = 20;
    cout << "Begin of experiment " << init_sequence << endl;
    
    EnergyMRTKernel::EMRTK_BALANCE_ENABLED                                  = 0;
    EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED                            = 0;
    EnergyMRTKernel::EMRTK_MIGRATE_ENABLED                                  = 1; // 1
    EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED                                = 0;
    EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME                        = 0;
    EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END                          = 0;

    EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END           = 1; // 1
    EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END                            = 1; // 1
    EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG            = 0; // 0
    EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG             = 1; // 1
    EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_LITTLE          = 1; // 1
    EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG_ONLY_FIRST  = 1;
    EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED                  = 1;

    EnergyMRTKernel::EMRTK_TRY_KEEP_OLD_CPU                                 = 1;
    EnergyMRTKernel::EMRTK_ENLARGE_WCET                                     = 1;
    EnergyMRTKernel::EMRTK_ENLARGE_WCET_10_PERC                             = 0; // 0
    EnergyMRTKernel::EMRTK_ENLARGE_WCET_UNIF                                = 1; // 1

    EnergyMRTKernel::EMRTK_ABORT_ON_NOT_SCHEDULABLE                         = 1;

    EnergyMRTKernel::EMRTK_FORGET_UACTIVE_ON_CORE_FREE                      = 1;
    EnergyMRTKernel::EMRTK_FREQUENCY_SCALE_DOWN                             = 1;

    EnergyMRTKernel::EMRTK_PLACEMENT_LITTLE_FIRST                           = 0; // 0

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);
    vector<CPU_BL *> cpus_little = kern->getIslandLittle()->getProcessors();
    vector<CPU_BL *> cpus_big = kern->getIslandBig()->getProcessors();
    vector<AbsRTTask*> tasks;
    // MissCount miss_count("miss");

    string  names[] = { "B0_killed", "B1", "B2", "B3_running", "B3_ready" };
    int     wcets[] = { 160, 450, 450, 400, 60 };
    int     deads[] = { 200, 500, 500, 500, 500 };
    vector<CBServerCallingEMRTKernel*> ets;
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        task_name = "T" + to_string(init_sequence) + "_task_BIG_" + names[j];
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(deads[j], deads[j], 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcets[j], workload.c_str());
        cout << instr << endl;
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));
        tasks.push_back(t);
        // miss_count.attachToTask(t);
    }
    EnergyMRTKernel* k = dynamic_cast<EnergyMRTKernel*>(kern);
    k->addForcedDispatch(ets[0], cpus_big[0], 18);
    k->addForcedDispatch(ets[1], cpus_big[1], 18);
    k->addForcedDispatch(ets[2], cpus_big[2], 18);
    k->addForcedDispatch(ets[3], cpus_big[3], 18);
    k->addForcedDispatch(ets[4], cpus_big[3], 18); // ready

    // for (CPU_BL* c : cpus_little)
    //     c->toggleDisabled();

    SIMUL.initSingleRun();
    SIMUL.run_to(1);
    k->printState(true);
    // return 0;
    
    SIMUL.run_to(150); // kill task on big0, task ready on big0 gets running
    cout << k->getScheduler()->toString() << endl;
    ets[0]->killInstance();
    SIMUL.sim_step(); // t=150, but all events have been processed
    cout << "u active big 0 " << k->getUtilization_active(cpus_big[0]) << endl;
    cout << "idle evt " << ets[0]->getIdleEvent() << endl;
    cout << "server status " << ets[0]->getStatusString() << endl;
    REQUIRE (k->getUtilization_active(cpus_big[0]) > 0.75); // shall be 0.8
    REQUIRE ( (double) ets[0]->getIdleEvent() >= 181);
    REQUIRE (ets[0]->getStatus() == ServerStatus::RELEASING);

    SIMUL.run_to(151);
    k->printState(true);
    // no migration, only schedule ready tasks
    REQUIRE (k->getRunningTask(cpus_big[0]) == NULL);
    REQUIRE (k->getReadyTasks(cpus_big[0]).size() == 0);
    REQUIRE (k->getUtilization_active(cpus_big[0]) > 0.75); // shall be 0.8

    SIMUL.run_to(182); // end vtime => migration
    REQUIRE (k->getRunningTask(cpus_big[0]) == ets[4]);
    REQUIRE (k->getUtilization_active(cpus_big[0]) == 0.0);
    REQUIRE (isInRange (cpus_big[0]->getUtilizationReadyRunning(), k->getCurrentUtilization(ets[4]) ) );
    cout << cpus_big[0]->getUtilizationReadyRunning() << endl;
    // return 0;

    cout << endl << "Scheduler state t=189:"<<endl;
    cout << k->getScheduler()->toString() << endl << endl;

    SIMUL.run_to(201);
    k->printState(true);
    REQUIRE (k->getRunningTask(cpus_big[0]) == ets[0]);
    REQUIRE (k->getReadyTasks(cpus_big[0]).at(0) == ets[4]);

    SIMUL.run_to(501); // all tasks are over, usual dispatch
    k->printState(true);
    for (CBServerCallingEMRTKernel *e : ets)
        REQUIRE (k->getProcessor(e) != NULL);


    SIMUL.endSingleRun();

    cout << "--------------" << endl;
    cout << "Simulation finished" << endl;
    for (AbsRTTask *t : tasks)
        delete t;
    for (CBServerCallingEMRTKernel* cbs : ets)
        delete cbs;
    delete k;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = Requisite(0,0,0,0);
}

TEST_CASE ("Experiment 23") {
    /// Does killInstance() on CBS server enveloping periodic tasks work?
    init_sequence = 23;
    cout << "Begin of experiment " << init_sequence << endl;
    Requisite req(false, true, false, true, true, false, true);
    if (!checkRequisites( req ))  return;

    EnergyMRTKernel *kern;
    init_suite(&kern);
    REQUIRE(kern != NULL);
    vector<CPU_BL *> cpus_little = kern->getIslandLittle()->getProcessors();
    vector<CPU_BL *> cpus_big = kern->getIslandBig()->getProcessors();
    vector<AbsRTTask*> tasks;

    int wcets[] = { 10 };
    int deads[] = { 200 };
    vector<CBServerCallingEMRTKernel*> ets;
    for (int j = 0; j < sizeof(wcets) / sizeof(wcets[0]); j++) {
        task_name = "T" + to_string(init_sequence) + "_task_BIG_" + to_string(j);
        cout << "Creating task: " << task_name;
        PeriodicTask* t = new PeriodicTask(deads[j], deads[j], 0, task_name);
        char instr[60] = "";
        sprintf(instr, "fixed(%d, %s);", wcets[j], workload.c_str());
        cout << instr << endl;
        t->insertCode(instr);
        ets.push_back(kern->addTaskAndEnvelope(t, ""));
        tasks.push_back(t);
    }
    EnergyMRTKernel* k = dynamic_cast<EnergyMRTKernel*>(kern);
    k->addForcedDispatch(ets[0], cpus_big[0], 18);

    for (CPU_BL* c : cpus_little)
        c->toggleDisabled();

    SIMUL.initSingleRun();

    SIMUL.run_to(1);
    cout << "==================" << endl;
    ets[0]->killInstance();
    k->printState(true);
    REQUIRE (k->getRunningTask(cpus_big[0]) == NULL);
    REQUIRE (k->getReadyTasks(cpus_big[0]).empty());
    REQUIRE (k->getUtilization_active(cpus_big[0]) > 0.0);
    SIMUL.run_to(201);
    cout << "==================" << endl;
    cout << "t=" << time() << endl;
    cout << "state of kernel:" << endl; k->printState(true);
    REQUIRE (k->getProcessorRunning(ets[0])->getIslandType() == IslandType::BIG);

    SIMUL.endSingleRun();

    cout << "--------------" << endl;
    cout << "Simulation finished" << endl;
    for (AbsRTTask *t : tasks)
        delete t;
    for (CBServerCallingEMRTKernel* cbs : ets)
        delete cbs;
    delete k;
    cout << "End of Experiment #" << init_sequence << endl << endl;
    performedTests[init_sequence] = req;
}



TEST_CASE("None") {
    for (const auto& elem : performedTests) {
        cout << "Performed experiment #" << elem.first << " with " << elem.second.toString() << endl;
    }
}

void getCores(vector<CPU_BL*> &cpus_little, vector<CPU_BL*> &cpus_big, Island_BL **island_bl_little, Island_BL **island_bl_big) {
    unsigned int OPP_little = 0; // Index of OPP in LITTLE cores
    unsigned int OPP_big = 0;    // Index of OPP in big cores

    vector<double> V_little = {
            0.92, 0.919643, 0.919357, 0.918924, 0.95625, 0.9925, 1.02993, 1.0475, 1.08445, 1.12125, 1.15779, 1.2075,
            1.25625
    };
    vector<unsigned int> F_little = {
            200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400
    };

    vector<double> V_big = {
            0.916319, 0.915475, 0.915102, 0.91498, 0.91502, 0.90375, 0.916562, 0.942543, 0.96877, 0.994941, 1.02094,
            1.04648, 1.05995, 1.08583, 1.12384, 1.16325, 1.20235, 1.2538, 1.33287
    };
    vector<unsigned int> F_big = {
            200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000
    };

    if (OPP_little >= V_little.size() || OPP_big >= V_big.size())
        exit(-1);

    unsigned long int max_frequency = max(F_little[F_little.size() - 1], F_big[F_big.size() - 1]);

    /* ------------------------- Creating CPU_BLs -------------------------*/
    for (unsigned int i = 0; i < 4; ++i) {
        /* Create 4 LITTLE CPU_BLs */
        string cpu_name = "LITTLE_" + to_string(i);

        cout << "Creating CPU_BL: " << cpu_name << endl;

        cout << "f is " << F_little[F_little.size() - 1] << " max_freq " << max_frequency << endl;

        CPUModelBP *pm = new CPUModelBP(V_little[V_little.size() - 1], F_little[F_little.size() - 1], max_frequency);
        {
            CPUModelBP::PowerModelBPParams idle_pp = {0.00134845, 1.76307e-5, 124.535, 1.00399e-10};
            CPUModelBP::ComputationalModelBPParams idle_cp = {1, 0, 0, 0};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("idle", idle_pp, idle_cp);

            CPUModelBP::PowerModelBPParams bzip2_pp = {0.00775587, 33.376, 1.54585, 9.53439e-10};
            CPUModelBP::ComputationalModelBPParams bzip2_cp = {0.0256054, 2.9809e+6, 0.602631, 8.13712e+9};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp, bzip2_cp);

            CPUModelBP::PowerModelBPParams hash_pp = {0.00624673, 176.315, 1.72836, 1.77362e-10};
            CPUModelBP::ComputationalModelBPParams hash_cp = {0.00645628, 3.37134e+6, 7.83177, 93459};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("hash", hash_pp, hash_cp);

            CPUModelBP::PowerModelBPParams encrypt_pp = {0.00676544, 26.2243, 5.6071, 5.34216e-10};
            CPUModelBP::ComputationalModelBPParams encrypt_cp = {6.11496e-78, 3.32246e+6, 6.5652, 115759};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp, encrypt_cp);

            CPUModelBP::PowerModelBPParams decrypt_pp = {0.00629664, 87.1519, 2.93286, 2.80871e-10};
            CPUModelBP::ComputationalModelBPParams decrypt_cp = {5.0154e-68, 3.31791e+6, 7.154, 112163};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp, decrypt_cp);

            CPUModelBP::PowerModelBPParams cachekiller_pp = {0.0126737, 67.9915, 1.63949, 3.66185e-10};
            CPUModelBP::ComputationalModelBPParams cachekiller_cp = {1.20262, 352597, 2.03511, 169523};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp, cachekiller_cp);
        }

        cout << "creating cpu" << endl;
        CPU_BL *c = new CPU_BL(cpu_name, "idle", pm);
        c->setIndex(i);
        pm->setCPU(c);
        pm->setFrequencyMax(max_frequency);
        //TracePowerConsumption *power_trace = new TracePowerConsumption(c, 1, "power_" + cpu_name + ".txt");
        //ptrace.push_back(power_trace);

        cpus_little.push_back(c);
    }

    for (unsigned int i = 0; i < 4; ++i) {
        /* Create 4 big CPU_BLs */

        string cpu_name = "BIG_" + to_string(i);

        cout << "Creating CPU_BL: " << cpu_name << endl;

        CPUModelBP *pm = new CPUModelBP(V_big[V_big.size() - 1], F_big[F_big.size() - 1], max_frequency);
        {
            CPUModelBP::PowerModelBPParams idle_pp = {0.0162881, 0.00100737, 55.8491, 1.00494e-9};
            CPUModelBP::ComputationalModelBPParams idle_cp = {1, 0, 0, 0};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("idle", idle_pp, idle_cp);

            CPUModelBP::PowerModelBPParams bzip2_pp = {0.0407739, 12.022, 3.33367, 7.4577e-9};
            CPUModelBP::ComputationalModelBPParams bzip2_cp = {0.17833, 1.63265e+6, 1.62033, 118803};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp, bzip2_cp);

            CPUModelBP::PowerModelBPParams hash_pp = {0.0388215, 16.3205, 4.3418, 5.07039e-9};
            CPUModelBP::ComputationalModelBPParams hash_cp = {0.017478, 1.93925e+6, 4.22469, 83048.3};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("hash", hash_pp, hash_cp);

            CPUModelBP::PowerModelBPParams encrypt_pp = {0.0348728, 8.14399, 5.64344, 7.69915e-9};
            CPUModelBP::ComputationalModelBPParams encrypt_cp = {8.39417e-34, 1.99222e+6, 3.33002, 96949.4};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp, encrypt_cp);

            CPUModelBP::PowerModelBPParams decrypt_pp = {0.0320508, 25.8727, 3.27135, 4.11773e-9};
            CPUModelBP::ComputationalModelBPParams decrypt_cp = {9.49471e-35, 1.98761e+6, 2.65652, 109497};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp, decrypt_cp);

            CPUModelBP::PowerModelBPParams cachekiller_pp = {0.086908, 9.17989, 2.5828, 7.64943e-9};
            CPUModelBP::ComputationalModelBPParams cachekiller_cp = {0.825212, 235044, 786.368, 25622.1};
            dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp, cachekiller_cp);
        }

        CPU_BL *c = new CPU_BL(cpu_name, "idle", pm);
        c->setIndex(i);
        pm->setCPU(c);
        pm->setFrequencyMax(max_frequency);
        //TracePowerConsumption *power_trace = new TracePowerConsumption(c, 1, "power_" + cpu_name + ".txt");
        //ptrace.push_back(power_trace);

        cpus_big.push_back(c);
    }


    vector<struct OPP> opps_little = Island_BL::buildOPPs(V_little, F_little);
    vector<struct OPP> opps_big = Island_BL::buildOPPs(V_big, F_big);
    *island_bl_little = new Island_BL("little island", IslandType::LITTLE, cpus_little, opps_little);
    *island_bl_big = new Island_BL("big island", IslandType::BIG, cpus_big, opps_big);
}

int init_suite(EnergyMRTKernel** kern) {
    cout << "init_suite" << endl;

    #if LEAVE_LITTLE3_ENABLED
        cout << "Error: tests thought for LEAVE_LITTLE3_ENABLED disabled" << endl;
        abort();
    #endif

    Island_BL *island_bl_big = NULL, *island_bl_little = NULL;
    vector<CPU_BL *> cpus_little, cpus_big;
    vector<Scheduler*> schedulers;
    vector<RTKernel*> kernels;

    getCores(cpus_little, cpus_big, &island_bl_little, &island_bl_big);
    REQUIRE(island_bl_big != NULL); REQUIRE(island_bl_little != NULL);
    REQUIRE(cpus_big.size() == 4); REQUIRE(cpus_little.size() == 4);

    EDFScheduler *edfsched = new EDFScheduler;
    for (int i = 0; i < 8; i++)
      schedulers.push_back(new EDFScheduler());

    *kern = new EnergyMRTKernel(schedulers, edfsched, island_bl_big, island_bl_little, "The sole kernel");
    kernels.push_back(*kern);

    CPU_BL::REFERENCE_FREQUENCY                                     = 2000; // BIG_3 frequency
    CPU_BL::REFERENCE_SPEED                                         = 1.0053736425473412;
    EDFScheduler::threshold                                         = 1.0;
    EnergyMRTKernel::EMRTK_ENLARGE_WCET                             = false;

    vector < CPU_BL * > cpus;
    cpus.reserve(cpus_little.size() + cpus_big.size());
    cpus.insert (cpus.end(), cpus_little.begin(), cpus_little.end());
    cpus.insert (cpus.end(), cpus_big.begin(), cpus_big.end()); 
    vector<Island_BL*> islands = { (*kern)->getIslandBig(), (*kern)->getIslandLittle() };
    for (CPU_BL* cpu : cpus) {
        cpu->setSynchPowerTracerDirectory("/dev/null/");
    }
    for (Island_BL* isl : islands) {
        isl->setSynchFreqTracerDirectory("/dev/null/");
    }

    cout << "end init_suite of Experiment #" << init_suite << endl;
    return 0;
}

/// Returns true if the value 'eval' and 'expected' are distant 'error'%
bool isInRange(int eval, int expected) {
    const unsigned int error = 5;

    int min = int(eval - eval * error/100);
    int max = int(eval + eval * error/100);

    return expected >= min && expected <= max; 
}

/// True if min <= eval <= max
bool isInRangeMinMax(double eval, const double min, const double max) {
    return min <= eval <= max;
}

bool checkRequisites(Requisite reqs) {
    cout << "Experiments with " << reqs.toString() << endl; 
    reqs.satisfied = false;

    if (FORCE_REQUISITES) {
        fixDependencies(reqs);

        cout << "Changing EMRTK policies settings as needed:" << endl;

        // edit here when you add a new policy in EnergyMRTKernel
        cout << "\tSetting Leave Little 3 free policy to " << reqs.EMRTK_leave_little3 << endl;
        EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED = reqs.EMRTK_leave_little3;

        cout << "\tSetting migration policies to " << reqs.EMRTK_migrate << endl;
        EnergyMRTKernel::EMRTK_MIGRATE_ENABLED = reqs.EMRTK_migrate;

        cout << "\tSetting CBS Server yielding policy to " << reqs.EMRTK_cbs_yield << endl;
        EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED = reqs.EMRTK_cbs_yield;

        cout << "\tSetting Temporarily Migrate on vtime end policy to " << reqs.EMRTK_temporarily_migrate_vtime << endl;
        EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME = reqs.EMRTK_temporarily_migrate_vtime;

        cout << "\tSetting Temporarily Migrate on end policy to " << reqs.EMRTK_temporarily_migrate_end << endl;
        EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END = reqs.EMRTK_temporarily_migrate_end;


        cout << "\tSetting CBS enveloping periodic tasks to " << reqs.EMRTK_cbs_enveloping_per_task_enabled << endl;
        EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED = reqs.EMRTK_cbs_enveloping_per_task_enabled;

        cout << "\tSetting CBS migration after tasks end to " << reqs.EMRTK_cbs_migrate_after_end << endl;
        EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END = reqs.EMRTK_cbs_migrate_after_end;

        cout << "\tSetting CBS migration after virtualtime end to " << reqs.EMRTK_cbs_enveloping_migrate_after_vtime_end << endl;
        EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END = reqs.EMRTK_cbs_enveloping_migrate_after_vtime_end;

        reqs.satisfied = true;
        return true;
    }

    // edit here when you add a new policy in EnergyMRTKernel
    if ( reqs.EMRTK_leave_little3 != EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED ) {
        cout << "Test requires EMRTK_LEAVE_LITTLE3_ENABLED = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED << ". Skip" << endl;
        return false;
    }
    if ( reqs.EMRTK_migrate != EnergyMRTKernel::EMRTK_MIGRATE_ENABLED ) {
        cout << "Test requires EMRTK_MIGRATE_ENABLED = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_MIGRATE_ENABLED << ". Skip" << endl;
        return false;
    }
    if (reqs.EMRTK_cbs_yield != EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED) {
        cout << "Test requires EMRTK_CBS_YIELD_ENABLED = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED << ". Skip" << endl;
        return false;
    }

    if (reqs.EMRTK_temporarily_migrate_vtime != EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME) {
        cout << "Test requires EMRTK_TEMPORARILY_MIGRATE_VTIME = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME << ". Skip" << endl;
        return false;
    }

    if (reqs.EMRTK_temporarily_migrate_end != EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END) {
        cout << "Test requires EMRTK_TEMPORARILY_MIGRATE_END = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END << ". Skip" << endl;
        return false;
    }



    if (reqs.EMRTK_cbs_enveloping_per_task_enabled != EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED) {
        cout << "Test requires EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED << ". Skip" << endl;
        return false;
    }
    if (reqs.EMRTK_cbs_enveloping_migrate_after_vtime_end != EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END) {
        cout << "Test requires EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END << ". Skip" << endl;
        return false;
    }
    if (reqs.EMRTK_cbs_migrate_after_end != EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END) {
        cout << "Test requires EMRTK_CBS_MIGRATE_AFTER_END = 1, but it's disabled: " << EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END << ". Skip" << endl;
        return false;
    }
    reqs.satisfied = true;
    return true;
}

// edit here when you add a new policy in EnergyMRTKernel
bool fixDependencies(Requisite reqs, bool abortOnFix) {
    cout << __func__ << "()" << endl;

    // CBS enveloping periodic tasks
    if (reqs.EMRTK_cbs_enveloping_migrate_after_vtime_end && !reqs.EMRTK_cbs_enveloping_per_task_enabled)
        if (abortOnFix) {
            cout << "\tabort on line " << __LINE__ << endl;
            exit(0);
        }
        else reqs.EMRTK_cbs_enveloping_per_task_enabled = 1;

    if (reqs.EMRTK_cbs_migrate_after_end && !reqs.EMRTK_cbs_enveloping_per_task_enabled)
        if (abortOnFix) {
            cout << "\tabort on line " << __LINE__ << endl;
            exit(0);
        }
        else reqs.EMRTK_cbs_enveloping_per_task_enabled = 1;


    // migrations
    if (reqs.EMRTK_cbs_enveloping_migrate_after_vtime_end && !reqs.EMRTK_migrate)
        if (abortOnFix) {
            cout << "\tabort on line " << __LINE__ << endl;
            exit(0);
        }
        else reqs.EMRTK_migrate = 1;

    if (reqs.EMRTK_cbs_migrate_after_end && !reqs.EMRTK_migrate)
        if (abortOnFix) {
            cout << "\tabort on line " << __LINE__ << endl;
            exit(0);
        }
        else reqs.EMRTK_migrate = 1;

    if (reqs.EMRTK_temporarily_migrate_vtime && !reqs.EMRTK_migrate)
        if (abortOnFix) {
            cout << "\tabort on line " << __LINE__ << endl;
            exit(0);
        }
        else reqs.EMRTK_migrate = 1;
}
