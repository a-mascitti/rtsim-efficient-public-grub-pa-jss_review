/***************************************************************************
    begin                : Thu Apr 24 15:54:58 CEST 2003
    copyright            : (C) 2003 by Giuseppe Lipari
    email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <algorithm>

#include <simul.hpp>

#include <cpu.hpp>
#include <mrtkernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <task.hpp>
#include <cbserver.hpp>

namespace RTSim
{

    using namespace std;
    using namespace MetaSim;

    template <class IT>
    void clean_mapcontainer(IT b, IT e)
    {
        for (IT i = b; i != e; i++)
            delete i->second;
    }

    BeginDispatchMultiEvt::BeginDispatchMultiEvt(MRTKernel &k, CPU &c)
        : DispatchMultiEvt(k, c, Event::_DEFAULT_PRIORITY + 10)
    {
    }

    EndDispatchMultiEvt::EndDispatchMultiEvt(MRTKernel &k, CPU &c)
        : DispatchMultiEvt(k, c, Event::_DEFAULT_PRIORITY + 10)
    {
    }

    void BeginDispatchMultiEvt::doit()
    {
        _kernel.onBeginDispatchMulti(this);
    }

    void EndDispatchMultiEvt::doit()
    {
        _kernel.onEndDispatchMulti(this);
    }

    vector<CPU *> MRTKernel::getProcessors() const
    {
        vector<CPU *> s; // = new vector<const CPU*>;

        typedef map<CPU *, AbsRTTask *>::const_iterator IT;
        int j = 0;

        for (IT i = _m_currExe.begin(); i != _m_currExe.end(); i++, j++)
            //s[j] = i->first;
            s.push_back(i->first);
        return s;
    }

    CPU *MRTKernel::getFreeProcessor()
    {
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); i++)
            if (i->second == NULL)
                return i->first;
        return NULL;
    }

    bool MRTKernel::isDispatched(CPU *p)
    {
        map<const AbsRTTask *, CPU *>::iterator j = _m_dispatched.begin();

        for (; j != _m_dispatched.end(); ++j)
            if (j->second == p)
                return true;

        return false;
    }

    MRTKernel::ITCPU MRTKernel::getNextFreeProc(ITCPU s, ITCPU e)
    {
        for (ITCPU i = s; i != e; ++i)
            if (i->second == NULL && !isDispatched(i->first))
                return i;

        return e;
    }

    void MRTKernel::internalConstructor(int n)
    {
        for (int i = 0; i < n; i++)
        {
            CPU *c = _CPUFactory->createCPU();
            _m_currExe[c] = NULL;
            _isContextSwitching[c] = false;
            _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
            _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
        }

        _sched->setKernel(this);
    }

    MRTKernel::MRTKernel(Scheduler *s, absCPUFactory *fact, int n,
                         const string &name)
        : RTKernel(s, name), _CPUFactory(fact), _migrationDelay(0)
    {
        internalConstructor(n);
    }

    MRTKernel::MRTKernel(Scheduler *s, std::vector<CPU *> cpus,
                         const string &name)
        : RTKernel(s, name), _migrationDelay(0)
    {
        for (CPU *c : cpus)
        {
            _m_currExe[c] = NULL;
            _isContextSwitching[c] = false;
            _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
            _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
        }

        _sched->setKernel(this);
    }

    MRTKernel::MRTKernel(Scheduler *s, int n, const string &name)
        : RTKernel(s, name), _migrationDelay(0)
    {
        _CPUFactory = new uniformCPUFactory();

        internalConstructor(n);
    }

    MRTKernel::MRTKernel(Scheduler *s, const string &name)
        : RTKernel(s, name), _migrationDelay(0)
    {
        _CPUFactory = new uniformCPUFactory();

        internalConstructor(1);
    }

    MRTKernel::~MRTKernel()
    {
        // delete _CPUFactory;
        clean_mapcontainer(_beginEvt.begin(), _beginEvt.end());
        clean_mapcontainer(_endEvt.begin(), _endEvt.end());
    }

    void MRTKernel::addCPU(CPU *c)
    {
        DBGENTER(_KERNEL_DBG_LEV);
        _m_currExe[c] = NULL;
        _isContextSwitching[c] = false;
        _beginEvt[c] = new BeginDispatchMultiEvt(*this, *c);
        _endEvt[c] = new EndDispatchMultiEvt(*this, *c);
    }

    void MRTKernel::addTask(AbsRTTask &t, const string &param)
    {
        RTKernel::addTask(t, param);
        _m_oldExe[&t] = NULL;
        _m_dispatched[&t] = NULL;

        CBServer *cbs = dynamic_cast<CBServer *>(&t);
        if (cbs != nullptr)
            _servers.push_back(cbs);
    }

    CPU *MRTKernel::getProcessor(const AbsRTTask *t) const
    {
        DBGENTER(_KERNEL_DBG_LEV);
        CPU *ret = NULL;

        typedef map<CPU *, AbsRTTask *>::const_iterator IT;

        for (IT i = _m_currExe.begin(); i != _m_currExe.end(); i++)
            if (i->second == t)
                ret = i->first;
        return ret;
    }

    CPU *MRTKernel::getOldProcessor(const AbsRTTask *t) const
    {
        CPU *ret = NULL;

        DBGENTER(_KERNEL_DBG_LEV);

        ret = _m_oldExe.find(t)->second;

        return ret;
    }

    void MRTKernel::suspend(AbsRTTask *task)
    {
        DBGENTER(_MRTKERNEL_DBG_LEV);

        _sched->extract(task);
        CPU *p = getProcessor(task);
        if (p != NULL)
        {
            task->deschedule();

            _m_currExe[p] = NULL;
            _m_oldExe[task] = p;
            _m_dispatched[task] = NULL;
            dispatch(p);
        }
    }

    void MRTKernel::onArrival(AbsRTTask *t)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        _sched->insert(t);

        dispatch();
    }

    // Task end event. Dispatch the task with closest deadline on the ending core.
    void MRTKernel::onEnd(AbsRTTask *task)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        CPU *p = getProcessor(task);

        if (p == NULL)
            throw RTKernelExc("Received a onEnd of a non executing task");

        _sched->extract(task);
        _m_oldExe[task] = p;
        _m_currExe[p] = NULL;
        _m_dispatched[task] = NULL;

        dispatch(p);
    }

    // A job instance arrives (its new period begins)
    void MRTKernel::dispatch()
    {
        DBGENTER(_KERNEL_DBG_LEV);

        int ncpu = _m_currExe.size();
        int num_newtasks = 0; // tells us how many "new" tasks in the
                              // ready queue
        int i;

        // take (#cores) tasks with closest deadline
        for (i = 0; i < ncpu; ++i)
        {
            AbsRTTask *t = _sched->getTaskN(i);
            if (t == NULL)
                break;
            else if (getProcessor(t) == NULL &&
                     _m_dispatched[t] == NULL)
                num_newtasks++;
        }

        _sched->print();
        DBGPRINT_2("New tasks: ", num_newtasks);
        print();
        if (num_newtasks == 0)
            return; // nothing to do

        ITCPU start = _m_currExe.begin();
        ITCPU stop = _m_currExe.end();
        ITCPU f = start;
        do
        {
            f = getNextFreeProc(f, stop);
            if (f != stop)
            {
                DBGPRINT_2("Dispatching on free processor ",
                           f->first);
                dispatch(f->first);
                num_newtasks--;
                f++;
            }
            else
            { // no more free processors
                // now we deschedule tasks, the ones with furthest deadlines first.
                // make an example with 2 cores. t=0 (4,5) and (2,3); at t=1, (3,4) arrives. (4,5) goes descheduled
                for (;;)
                {
                    AbsRTTask *t = _sched->getTaskN(i++);
                    if (t == NULL)
                        throw RTKernelExc("Can't find enough tasks to deschedule!");

                    CPU *c = _m_dispatched[t];
                    if (c != NULL)
                    {
                        DBGPRINT_4("Dispatching on processor ", c,
                                   " which is executing task ", taskname(t));

                        dispatch(c);
                        num_newtasks--;
                        break;
                    }
                }
            }
        } while (num_newtasks > 0);
    }

    void MRTKernel::dispatch(CPU *p)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        if (p == NULL)
            throw RTKernelExc("Dispatch with NULL parameter");

        DBGPRINT_2("dispatching on processor ", p);
        _beginEvt[p]->drop();

        if (_isContextSwitching[p])
        {
            DBGPRINT("Context switch is disabled!");
            _beginEvt[p]->post(_endEvt[p]->getTime());
            AbsRTTask *task = _endEvt[p]->getTask();
            _endEvt[p]->drop();
            if (task != NULL)
            {
                _endEvt[p]->setTask(NULL);
                _m_dispatched[task] = NULL;
            }
        }
        else
            _beginEvt[p]->post(SIMUL.getTime());
    }

    // Take task with closest deadline and do context switch on the required core
    void MRTKernel::onBeginDispatchMulti(BeginDispatchMultiEvt *e)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        // if necessary, deschedule the task.
        CPU *p = e->getCPU();
        AbsRTTask *dt = _m_currExe[p];
        AbsRTTask *st = NULL;

        if (dt != NULL)
        {
            _m_oldExe[dt] = p;
            _m_currExe[p] = NULL;
            _m_dispatched[dt] = NULL;
            dt->deschedule();
        }

        // select the first non dispatched task in the queue.
        // This is the one with closest deadline.
        int i = 0;
        while ((st = _sched->getTaskN(i)) != NULL)
            if (_m_dispatched[st] == NULL)
                break;
            else
                i++;

        if (st == NULL)
        {
            DBGPRINT("Nothing to schedule, finishing");
        }

        DBGPRINT_4("Scheduling task ", taskname(st), " on cpu ", p->toString());

        if (st)
            _m_dispatched[st] = p;
        _endEvt[p]->setTask(st);
        _isContextSwitching[p] = true;
        Tick overhead(_contextSwitchDelay);
        if (st != NULL && _m_oldExe[st] != p && _m_oldExe[st] != NULL)
            overhead += _migrationDelay;
        _endEvt[p]->post(SIMUL.getTime() + overhead);
    }

    void MRTKernel::onEndDispatchMulti(EndDispatchMultiEvt *e)
    {
        // performs the "real" context switch
        DBGENTER(_KERNEL_DBG_LEV);

        AbsRTTask *st = e->getTask();
        CPU *p = e->getCPU();

        _m_currExe[p] = st;

        DBGPRINT_2("CPU: ", p->toString());
        DBGPRINT_2("Task: ", taskname(st));
        ECOUT (endl);
        printState();
        ECOUT (endl);

        // st could be null (because of an idling processor)
        if (st)
            st->schedule();

        _isContextSwitching[p] = false;
        _sched->notify(st);
    }

    void MRTKernel::printState()
    {
        Entity *task;
        ECOUT("MRTKernel::printstate(), time " << SIMUL.getTime() << " ");
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); i++)
        {
            task = dynamic_cast<Entity *>(i->second);
            if (task != NULL)
                ECOUT(i->first->getName() << " : " << task->getName() << "   ");
            else
                ECOUT(i->first->getName() << " :   0   ");
        }
        ECOUT(endl);
    }

    void MRTKernel::newRun()
    {
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); i++)
        {
            if (i->second != NULL)
                _sched->extract(i->second);
            i->second = NULL;
        }
        map<const AbsRTTask *, CPU *>::iterator j = _m_dispatched.begin();
        for (; j != _m_dispatched.end(); ++j)
            j->second = NULL;

        j = _m_oldExe.begin();
        for (; j != _m_oldExe.end(); ++j)
            j->second = NULL;
    }

    void MRTKernel::endRun()
    {
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); i++)
        {
            if (i->second != NULL)
                _sched->extract(i->second);
            i->second = NULL;
        }
    }

    void MRTKernel::print()
    {
        DBGPRINT("Executing");
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); ++i)
            DBGPRINT_4("  [", i->first, "] --> ", taskname(i->second));
        map<const AbsRTTask *, CPU *>::iterator j = _m_dispatched.begin();
        DBGPRINT("Dispatched");
        for (; j != _m_dispatched.end(); ++j)
            DBGPRINT_4("  [", taskname(j->first), "] --> ", j->second);
    }

    AbsRTTask *MRTKernel::getTask(CPU *c)
    {
        return _m_currExe[c];
    }

    std::vector<std::string> MRTKernel::getRunningTasks()
    {
        std::vector<std::string> tmp_ts;
        for (auto i = _m_currExe.begin(); i != _m_currExe.end(); i++)
        {
            std::string tmp_name = taskname((*i).second);
            if (tmp_name != "(nil)")
                tmp_ts.push_back(tmp_name);
        }
        return tmp_ts;
    }

    // ----------------------------------------------- MRTKernel_Linux5_3_11

    void MRTKernel_Linux5_3_11::dispatch()
    {
        DBGENTER(_KERNEL_DBG_LEV);
        ECOUT ("t=" << SIMUL.getTime() << " " << __func__ << endl);

        // A task instance arrives (its period begins).
        // Find a free CPU and dispatch the task there.
        // If all CPUs have one running task, 
        // then select the CPU
        // with latest deadline and preempt it.
        // If the task has the latest deadline wrt
        // the already dispatched ones, then suspend it.

        int num_newtasks = 0; // tells us how many "new" tasks in the
                              // ready queue
        vector<CPU *> processors = getProcessors();
        int ncpu = processors.size();

        // take (#cores) tasks with closest deadline
        for (int i = 0; i < ncpu; i++)
        {
            AbsRTTask *t = _sched->getTaskN(i);
            if (t == NULL)
                break;
            else if (getProcessor(t) == NULL &&
                     _m_dispatched[t] == NULL)
                num_newtasks++;
        }

        _sched->print();
        DBGPRINT_2("New tasks: ", num_newtasks);
        print();
        if (num_newtasks == 0)
            return; // nothing to do

        // CPU ordered by task deadline (later first)
        // CPU2: dl=1000, CPU3: dl=500, CPU1: dl=100
        vector<CPU *> laterCPUs;
        for (auto busyCPU : _m_currExe)
        {
            laterCPUs.push_back(busyCPU.first);
        }
        sort(laterCPUs.begin(), laterCPUs.end(),
             [this](CPU *e1, CPU *e2) {
                 if (_m_currExe.at(e1) == NULL && _m_currExe.at(e2) == NULL)
                     return false;
                 else if (_m_currExe.at(e1) == NULL)
                     return false;
                 else if (_m_currExe.at(e2) == NULL)
                     return true;
                 else
                     return _m_currExe.at(e1)->getDeadline() > _m_currExe.at(e2)->getDeadline();
             });

        vector<CPU *> freeProcessors;
        for (CPU *c : processors)
            if (!isDispatched(c))
                freeProcessors.push_back(c);

        unsigned int nDispatchedTasksOnFreeCPU = 0;
        for (CPU *c : freeProcessors)
        {
            if (num_newtasks <= 0)
                break;
            MRTKernel::dispatch(c);
            nDispatchedTasksOnFreeCPU++;
            num_newtasks--;

            AbsRTTask *rtask = getTask(c);
            ECOUT("t=" << SIMUL.getTime()
                       << " dispatch on free core: "
                       << c->toString()
                       << ", now running: "
                       << (rtask == NULL ? "none" : rtask->toString())
                       << endl);
        }

        // tasks with highest priority preempt a busy CPU
        int laterCPUsIndex = 0;
        for (int i = nDispatchedTasksOnFreeCPU; i < num_newtasks; i++)
        {
            CPU *laterCPU = laterCPUs.at(laterCPUsIndex);
            MRTKernel::dispatch(laterCPU);
            laterCPUsIndex++;

            AbsRTTask *rtask = getTask(laterCPU);
            string s = (rtask == NULL ? "none" : rtask->toString());
            ECOUT("t=" << SIMUL.getTime()
                       << " preemption on "
                       << laterCPU->toString()
                       << ", now running: "
                       << s
                       << endl);
        }
    }
    
    void MRTKernel_Linux5_3_11::printState(bool alsoCBS)
    {
        for (const auto &cpu : getProcessors())
        {
            AbsRTTask *rtask = getTask(cpu);
            string s = (rtask == NULL ? "none" : rtask->toString());
            ECOUT(cpu->toString()
                  << " -> "
                  << s
                  << endl);
        }

        if (alsoCBS)
            for (const auto &cbs : getServers())
                ECOUT(cbs->toString() << endl);
    }

} // namespace RTSim
