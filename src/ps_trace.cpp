#include <ps_trace.hpp>

#include <cbserver.hpp>


namespace RTSim {

    using namespace std;
    using namespace MetaSim;


    PSTrace::PSTrace(const string& name)
    {
        fd.open(name.c_str());
        first_event = true;
    }

    PSTrace::~PSTrace() {
        fd.close();
    }

    void PSTrace::writeTaskEvent(const Task &tt, const std::string &evt_name, TaskEvt* evt)
    {
        if (SIMUL.getTime() < startingTick) return;

        fd << SIMUL.getTime() << "\t";
        fd << tt.getName() << "\t";
        fd << (dynamic_cast<ArrEvt*>(evt) != NULL ? 0 : evt->getCPU()) << "\t";
        fd << evt_name << "\t";
        fd << endl;
    }

    void PSTrace::write(const AbsTask* t, const std::string &str, bool isTimeCheckEnabled)
    {
        if (isTimeCheckEnabled && startingTick <= SIMUL.getTime()) {
            const Task* tt = dynamic_cast<const Task*>(t); 
            fd << SIMUL.getTime() << "\t";
            fd << tt->getName() << "\t";
            fd << str << endl;
        }
    }

    void PSTrace::writeTasks(vector<CBServer*> tasks, vector<AbsRTTask*> tasksinside, string filename) {
        std::ofstream fdd;
        fdd.open(filename.c_str());

        for (int i = 0; i < tasks.size(); i++) {
            CBServer* t = tasks.at(i);
            PeriodicTask* tt = dynamic_cast<PeriodicTask*>(tasksinside.at(i));
            fdd << tt->getName() << " " << t->getBudget() << " "
                << tt->getWCET(1.0) << " " << tt->getPeriod() << " "
                << tt->getRelDline()
                << endl;
        }

        fdd.close();             
    }

    void PSTrace::writeCPUs(vector<Island_BL*> islands, string filename) {
        std::ofstream fdd;
        fdd.open(filename.c_str());

        for (Island_BL* isl : islands) {
            fdd << "BEGIN ISLAND " << (isl->getIslandType() == BIG ? "big" : "LITTLE") << endl;
            for (CPU_BL* c : isl->getProcessors())
                fdd << c->getIndex() << endl;
            fdd << "END ISLAND" << endl;
        }

        fdd.close();
    }

    void PSTrace::probe(ArrEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "CREATION\tI", &e);
    }

    void PSTrace::probe(EndEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tE", &e);
    }

    void PSTrace::probe(SchedEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tS", &e);
    }

    void PSTrace::probe(DeschedEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "RUNNING\tE", &e);
    }

    void PSTrace::probe(DeadEvt& e)
    {
        Task& tt = *(e.getTask());
        writeTaskEvent(tt, "MISS\t\tI", &e);
    }

    void PSTrace::attachToTask(Task& t)
    {
        attach_stat(*this, t.arrEvt);
        attach_stat(*this, t.endEvt);
        attach_stat(*this, t.schedEvt);
        attach_stat(*this, t.deschedEvt);
        attach_stat(*this, t.deadEvt);
    }
}

