 /***************************************************************************
 *   begin                : Thu Apr 24 15:54:58 CEST 2003
 *   copyright            : (C) 2003 by Agostino Mascitti
 *   email                : a.mascitti@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "energyMRTKernel.hpp"
#include "task.hpp"
#include "scheduler.hpp"
#include <assert.h>
#include "rttask.hpp"
#include "exeinstr.hpp"
#include "cpu.hpp"
#include "cbserver.hpp"

#include <sys/time.h>

int64_t BEGTICK = 0;

namespace RTSim {
    using namespace MetaSim;

    // Configuration for tests:
    bool EnergyMRTKernel::EMRTK_BALANCE_ENABLED                                 = 1; /* Can't imagine disabling it, but so policy is in the list :) */
    bool EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED                           = 0;
    bool EnergyMRTKernel::EMRTK_MIGRATE_ENABLED                                 = 1;
    bool EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED                               = 0;

    bool EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END                           = 0;
    bool EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG           = 0; // 0
    bool EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG            = 1; // 1
    bool EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG_ONLY_FIRST = 1;
    bool EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_LITTLE         = 0; // 1. I put 0 for tests
    bool EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME                       = 1;
    bool EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END                         = 1;

    bool EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED                 = 1;
    bool EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END          = 1;

    bool EnergyMRTKernel::EMRTK_ENLARGE_WCET                                    = 1;
    bool EnergyMRTKernel::EMRTK_ENLARGE_WCET_10_PERC                            = 0; // 0
    bool EnergyMRTKernel::EMRTK_ENLARGE_WCET_UNIF                               = 1; // 1
    bool EnergyMRTKernel::EMRTK_TRY_KEEP_OLD_CPU                                = 0;

    bool EnergyMRTKernel::EMRTK_DISABLE_DEBUG_TRYONCPU_BL                       = 0;

    bool EnergyMRTKernel::EMRTK_ABORT_ON_NOT_SCHEDULABLE                        = 0; // 0, simply current skip period

    bool EnergyMRTKernel::EMRTK_FORGET_UACTIVE_ON_CORE_FREE                     = 0;
    bool EnergyMRTKernel::EMRTK_FREQUENCY_SCALE_DOWN                            = 0;

    bool EnergyMRTKernel::EMRTK_PLACEMENT_LITTLE_FIRST                          = 0; // 0

    bool EnergyMRTKernel::EMRTK_FIT_BIG_LITTLE_CHOSEN                           = 1; // 0 att., reduced performane placem. alg.

    bool EnergyMRTKernel::EMRTK_PRINT_PLACEMENT_TIME                            = 1; // 1 att., reduces performance
    bool EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_WF                              = 1; // default placement logic is Worst-Fit
    bool EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_BF                              = 0;
    bool EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_FF                              = 0;

    EnergyMRTKernel::EnergyMRTKernel(vector<Scheduler*> &qs, Scheduler *s, Island_BL* big, Island_BL* little, const string &name)
      : MRTKernel(s, big->getProcessors().size() + little->getProcessors().size(), name) {
        setIslandBig(big); setIslandLittle(little);
        big->setKernel(this); little->setKernel(this);

        for(CPU_BL* c : getProcessors())  {
            _m_currExe[c] = NULL;
            _isContextSwitching[c] = false;
        }

        _sched->setKernel(this);

        vector<CPU_BL*> cpus = getProcessors();
        vector<CPU*> v;
        for (CPU_BL* c : cpus)
            v.push_back((CPU*) c);

        _queues = new EnergyMultiCoresScheds(this, v, qs, "energymultischeduler");
    }

    EnergyMultiCoresScheds::EnergyMultiCoresScheds(MRTKernel *kernel, vector<CPU*> &cpus, vector<Scheduler*> &s, const string& name)
      : MultiCoresScheds(kernel, cpus, s, kernel->getName() + name) { 

    }

    EnergyMRTKernel* EnergyMultiCoresScheds::getKernel() const { return dynamic_cast<EnergyMRTKernel*>(_kernel); }

    string EnergyMultiCoresScheds::toString(CPU_BL* c, int* executingTasks) const {
        vector<AbsRTTask*> tasks = getAllTasksInQueue(c);
        std::reverse(tasks.begin(), tasks.end());
        stringstream ss;
        int i = 1;
        for (AbsRTTask* t : tasks) {
            CBServerCallingEMRTKernel* cbs = dynamic_cast<CBServerCallingEMRTKernel*>(t);

            stringstream aec;
            if ( cbs->getStatus() == ServerStatus::READY)
                aec << ", alreadyExecdCycles=" << cbs->getAlreadyExecutedCycles();

            ss << "\t\t" << i++ << ") " << t->toString() << ", abs_dline=" << t->getDeadline() << aec.str() << ", endEvt=" << cbs->getEndEventTime() << endl;

            // for each queue there can be only 1 executing CBS server
            if (cbs != NULL && cbs->getStatus() == ServerStatus::EXECUTING)
              (*executingTasks)++;
        }
        return ss.str();
    }

    string EnergyMultiCoresScheds::toString() {
        int executingTasks = 0;
        stringstream ss;
        ss << "EnergyMultiCoresScheds::toString(), t=" << SIMUL.getTime() << ":" << endl;
        for (const auto& q : _queues) {
            string original = q.first->getWorkload();
            q.first->setWorkload("bzip2");
            executingTasks = 0;
            string qs = toString(dynamic_cast<CPU_BL *>(q.first), &executingTasks);
            string isMax = dynamic_cast<CPU_BL *>(q.first)->hasMaxUtilization() ? " <-- max" : "";
            string isMin = dynamic_cast<CPU_BL *>(q.first)->hasMinUtilization() ? " <-- min" : "";
            if (qs == "")
                ss << "\tt=" << SIMUL.getTime() << ", " << q.first->getName() << " ( freq: " << q.first->getFrequency() << ", speed: " << q.first->getSpeed() << ", (unscaled) u_active: " << dynamic_cast<CPU_BL*>(q.first)->getUtilization_active() << ", (unscaled) u_cur: " << dynamic_cast<CPU_BL*>(q.first)->getUtilizationReadyRunning() << ", isBusy: " << (dynamic_cast<CPU_BL*>(q.first)->isBusy() ? "busy" : "free") << ", wl: " << original << " ):" << isMax << isMin << endl 
                  << "\tempty queue" << endl << endl;
            else {
                ss << "\tt=" << SIMUL.getTime() << ", " << q.first->getName() << " ( freq: " << q.first->getFrequency() << ", speed: " << q.first->getSpeed() << ", (unscaled) u_active: " << dynamic_cast<CPU_BL *>(q.first)->getUtilization_active() << ", (unscaled) u_cur: " << dynamic_cast<CPU_BL *>(q.first)->getUtilizationReadyRunning() << ", isBusy: " << (dynamic_cast<CPU_BL *>(q.first)->isBusy() ? "busy" : "free") << ", wl: " << original << " ):" << isMax << isMin << endl;
                ss << qs << endl;
            }

            if (executingTasks > 1) {
                ECOUT( ss.str() << endl );
                assert (executingTasks <= 1);
            }
            q.first->setWorkload(original);
        }
        return ss.str();
    }

    void EnergyMultiCoresScheds::insertTask(AbsRTTask* t, CPU* c) {
        CPU_BL* cc = dynamic_cast<CPU_BL*>(c);
        ECOUT( "t=" << SIMUL.getTime() << " EMRTK::" << __func__ << "() " << endl );
        MultiCoresScheds::insertTask(t, c); // add job to c's scheduler

        double curUtil = getKernel()->getUtilization(t); // task curr util in cycles
        c->increaseUtilReadyRunning(curUtil);
        cc->setBusy(true);
    }

  void EnergyMultiCoresScheds::onMigrationFinished(AbsRTTask* t, CPU* original, CPU* final) {
        CBServerCallingEMRTKernel *cbs = dynamic_cast<CBServerCallingEMRTKernel*>(t);
        CPU_BL* o = dynamic_cast<CPU_BL*>(original), *f = dynamic_cast<CPU_BL*>(final);
        assert(cbs != NULL); assert(original != NULL); assert(final != NULL);
        ECOUT( "\t\tt=" << SIMUL.getTime() << " EMCS::" << __func__ << "()" << ", " << t->toString() << endl );

        // functions order is important below!
        original->decreaseTotalNomUtil(getKernel()->getUtilization(t), false); // cur util task, not adjust original island freq
        
        cbs->onMigrated(f); // update (increase) task nominal utilization according to its remaining time
        MultiCoresScheds::onMigrationFinished(t, original, final); // removes task from a queue and inserts it into the other, updating final core util & scheduling t
        
        // now the island has got its new frequency and thus you can assign the CBS new speed
        final->setWorkload(cbs->getTaskWorkload());
        cbs->changeBudget(final->getSpeed(), final);

        schedule(final);
    }



    // --------------------------------------------- EnergyMRTKernel methods

    AbsRTTask* EnergyMRTKernel::getRunningTask(CPU* c) const {
        AbsRTTask* t = _queues->getRunningTask(c);

        if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED)
            t = getEnveloper(t);

        return t;
    }

    vector<AbsRTTask*> EnergyMRTKernel::getReadyTasks(CPU_BL* c) const {
        vector<AbsRTTask*> t = _queues->getReadyTasks(c);
        vector<AbsRTTask*> tt;

        if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED)
            tt = getEnvelopers(t);
        else
            tt = t;

        for (AbsRTTask *a : tt)
            assert (NULL != dynamic_cast<CBServerCallingEMRTKernel*>(a));

        return tt;
    }

    CPU_BL *EnergyMRTKernel::getProcessorReady(AbsRTTask *t) const {
        if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED && dynamic_cast<PeriodicTask*>(t))
            t = getEnveloper(t);

        CPU_BL* c = dynamic_cast<CPU_BL*>(_queues->isInAnyQueue(t));
        if (_queues->getRunningTask(c) == t)
            c = NULL;

        return c;
    }

    double EnergyMRTKernel::getResCapNomCore(CPU_BL* cpu, bool isNom) const
    {
        string  orig = cpu->getWorkload();
        cpu->setWorkload("bzip2");

        double  coreUtilNom  = cpu->getUtilizationReadyRunning() + cpu->getUtilization_active(),
                coreSpeed    = cpu->getSpeed(); // non nominal

        if (isNom)
            coreSpeed = cpu->getIsland()->getUtilizationLimit(); // max OPP speed (1.0 on big, 0.345328 on LITTLE)
        
        cpu->setWorkload(orig);
        double coreResCap = coreSpeed - coreUtilNom;
        return coreResCap;
    }

    // this wrapping method's needed for classes inheritance structure (AbsRTTask <- CBServer; AbsRTTask <- PeriodicTask)
    double EnergyMRTKernel::getUtilization(AbsRTTask* t) const {
        CBServer* cbs = dynamic_cast<CBServer*>(t);
        double util_cur = cbs->getUtilization();
        return util_cur;
    }

    bool EnergyMRTKernel::isAdmissible(AbsRTTask* t, CPU_BL* c) const {
        assert (c != NULL); assert (t != NULL);
        double cur_util_rr  = c->getUtilizationReadyRunning(); // u_nom / REFERENCE_SPEED
        double u_active = c->getUtilization_active();
        double cur_util = cur_util_rr + u_active;
        assert (cur_util_rr <= 1.0); assert (u_active <= 1.0); assert (cur_util <= CPU_BL::REFERENCE_SPEED);
        assert (cur_util_rr >= -0.00000001); assert (u_active >= -0.00000001);

        double task_util = getUtilization(t); // u_nom / REFERENCE_SPEED

        double new_util  = cur_util + task_util;

        return new_util <= EDFScheduler::threshold;
    }

    void EnergyMRTKernel::onOppChanged(unsigned int oldOPP, unsigned int curOPP, Island_BL* island) {
        streambuf *old = cout.rdbuf();
        cout.rdbuf(NULL);
        ECOUT ("+++++++++++++" << endl);
        
        // update budget of servers (enveloping periodic tasks) in the island
        for (auto &elem : _envelopes) {
            CPU_BL *c = getProcessorReadyRunning(elem.first);

            // dispatch() dispatches a task per time, setting CPU OPP => some tasks don't have core yet
            if (c == NULL || elem.second->isKilled())
                continue;

            string startingWL = c->getWorkload();
            c->setWorkload(getEnveloper(elem.first)->getTaskWorkload());

            if (c->getIslandType() == island->getIslandType()) {
                Tick taskWCET = Tick::ceil(elem.first->getWCET(c->getSpeed()));
                double newSpeed = c->getSpeed(curOPP);
                ECOUT( __func__ << "(). changing budget to " << elem.second->toString() << " to " << taskWCET << ". core: " << c->toString() << " speed:" << c->getSpeed() << endl );

                elem.second->changeBudget(newSpeed, c);
                if (elem.second->getStatus() == ServerStatus::EXECUTING && getEnveloper(elem.second)->getEndEventTime() != time()) {
                	ECOUT( "endevt " << elem.second->toString() << " " << getEnveloper(elem.second)->getEndEventTime() << endl);
                    elem.second->getFirstTask()->updateEndEvent();
                }
            }

            c->setWorkload(startingWL);
        }

        // scale wcets of tasks on the island
        
        ECOUT ("+++++++++++++" << endl);
        cout.rdbuf(old);
    }


    // ----------------------------------------------------------- testing

    void EnergyMRTKernel::test() {
        // PeriodicTask T7_task0 DL = T 500 WCET(abs) 63 in CPU LITTLE_0 freq 500 freq 3
        AbsRTTask *t = dynamic_cast<AbsRTTask*>(_sched->getTaskN(0));
        CPU_BL* p;
        dispatch(t, p);
        p->setWorkload("bzip2");
        ECOUT( "CPU is " << p->toString() << " freq " << p->getFrequency()<< " "<< t->toString() << endl );

        int nTaskIsland = 0;

        exit(0);
    }

    // for gdb
    void EnergyMRTKernel::printBool(bool b) {
        ECOUT(  b << endl );
    }

    void EnergyMRTKernel::printState(bool alsoQueues, bool alsoCBSStatus/*=false*/, bool alsoRunningTasks/*=false*/) {
        if (_queues == NULL)
            return;

        ECOUT( "t=" << SIMUL.getTime() << ", printing scheduler state. " << "Running tasks:" << endl << "\t" );
        for (CPU_BL *c : getProcessors()) {
            AbsRTTask *t = _queues->getRunningTask(c);
            if (dynamic_cast<CBServer*>(t) && dynamic_cast<CBServer*>(t)->isYielding())
                t = _queues->getFirstReady(c);
            if (alsoRunningTasks)
                ECOUT( c->getName() << ": " << (t == NULL ? "0" : taskname(t)) << "\t" );
        }

        if (alsoQueues)
            ECOUT( endl << "Queues:" << endl << _queues->toString() << endl );

        if (alsoCBSStatus) {
            ECOUT( "CBS servers (= periodic tasks) statuses:" << endl );
            for (const auto& elem : _envelopes) {
                ECOUT( "- " << elem.second->toString() << endl );
            }
        }

        ECOUT( endl );
    }

    void EnergyMRTKernel::printAllTasks(string filename) const
    {
        ofstream ff(filename);
        for (const auto& env : _envelopes)
            ff << env.second->toString() << endl;
        ff.close();
    }

    void EnergyMRTKernel::addForcedDispatch(AbsRTTask* t, CPU_BL* c, unsigned int opp, unsigned int times) {
        _m_forcedDispatch[t] = make_tuple(c, opp, times);
    }

    // for gdb
    bool EnergyMRTKernel::manageForcedDispatch(AbsRTTask* t) {
        if (_m_forcedDispatch.find(t) != _m_forcedDispatch.end() ) {
            ECOUT( __func__ << endl );
            CPU_BL* c = get<0>(_m_forcedDispatch[t]);

            if (c->getUtilizationReadyRunning() + getUtilization(t) > 1.0) {
              cerr << "Cannot assign " << t->toString() << " u_nom=" << getUtilization(t) << " to the desired core " << c->getName() << " since new nom util > 1" << endl;
              printState(1);
              abort();
            }

            dispatch(t, c);
            
            get<2>(_m_forcedDispatch[t])--;
            if (get<2>(_m_forcedDispatch[t]) == 0)
               _m_forcedDispatch.erase(t);
            return true;
        }
        return false;
    }

    string EnergyMRTKernel::time() {
        return to_string(double(SIMUL.getTime()));
    }

    // --------------------------------------------------------------- context switch

    bool EnergyMRTKernel::isToBeDescheduled(CPU_BL *p, AbsRTTask *t) {
      assert(p != NULL);

      if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED && dynamic_cast<PeriodicTask*>(t))
            t = getEnveloper(t);

      if (dynamic_cast<RRScheduler*>(_sched) && t != NULL) { // t is null at time == 0
        bool res = false;
        try {
          res = dynamic_cast<RRScheduler*>(_queues->getScheduler(p))->isRoundExpired(t);
        } catch (BaseExc& e) { /* */ }
        return res;
      }
      return false; // neutral...
    }

    // Note MRTKernel version differs: dispatch() gives tasks a free CPU and calls onBDM(), which in turns
    // assigns them. EnergyMRTKernel, instead, needs to make assignment decisions: dispatch() chooses
    // a CPU for all tasks, and on*DM() makes the context switch (split into onBDM() and onEBM(), as in MRTKernel)
    // Begins context switch on a core, task is at the end of onBDM() still ready on core.
    void EnergyMRTKernel::onBeginDispatchMulti(BeginDispatchMultiEvt* e) {
        DBGENTER(_KERNEL_DBG_LEV);

        CPU_BL    *p    = dynamic_cast<CPU_BL*>(e->getCPU());
        AbsRTTask *dt   = _queues->getRunningTask(p);
        AbsRTTask *st   = e->getTask();
        e->drop();
        assert(st != NULL); assert(p != NULL);

        if ( st != NULL && dt == st ) {
          string ss = "Decided to dispatch " + st->toString() + " on its former CPU (" + p->toString() + ") => skip context switch";
            DBGPRINT(ss);
            ECOUT( "t=" << SIMUL.getTime() << " " << ss << endl );
            //return; // ctx switch finishes here (i.e. not done)
            assert (false); // shall never happen
        }
        // if necessary, deschedule the task.
        if ( dt != NULL || isToBeDescheduled(p, dt) ) {
            _m_oldExe[dt] = p;
            _m_currExe[p] = NULL;
            dt->deschedule();
        }

        DBGPRINT_4("Scheduling task ", taskname(st), " on cpu ", p->toString());

        _isContextSwitching[p] = true;
        Tick overhead (_contextSwitchDelay);
        CPU_BL* oldProcessor = dynamic_cast<CPU_BL*>(getOldProcessor(st));
        if (st != NULL && oldProcessor != p && oldProcessor != NULL)
            overhead += _migrationDelay;
        
        _queues->onBeginDispatchMultiFinished(p, st, overhead);

    }

    /**
        Called after dispatch(), i.e. after choosing a CPU forall arrived tasks.
        Here task begins executing in its core. End of context switch on core.
      */
    void EnergyMRTKernel::onEndDispatchMulti(EndDispatchMultiEvt* e) {
        AbsRTTask* t      = e->getTask();
        CPU_BL* cpu       = dynamic_cast<CPU_BL*>(e->getCPU());
        assert (t != NULL); assert(cpu != NULL);

        _queues->onEndDispatchMultiFinished(cpu,t);
        MRTKernel::onEndDispatchMulti(e);

        cpu->setBusy(true); // introduced afterwards

        // use case: 2 tasks arrive at t=0 and are scheduled on big 0 and big 1 freq 1100.
        // Then, at time t=100, another task arrives and the algorithm decides to schedule it on big 2 freq 2000.
        // Thus, big island freq is 2000, not 1100 (the max)
        unsigned int opp = _queues->getOPP(cpu);

        _m_oldExe[t] = cpu;

        ECOUT( endl );

        onTaskGetsRunning(t, cpu); // sets wl=bzip2
    }

    // --------------------------------------- job events

    void EnergyMRTKernel::onReplenishment(CBServerCallingEMRTKernel *cbs) {
        ECOUT( "EMRTK::" << __func__ << "()" << endl );
        CPU* cc = _queues->onReplenishment(cbs); // forgetU_active()
        CPU_BL* endingCPU = dynamic_cast<CPU_BL*>(cc);
        assert (endingCPU != NULL);

        endingCPU->forgetUtilization_active(cbs); // don't adjust frequency

        dispatch();
    }

    /// Callback, when a task gets running on a core
    void EnergyMRTKernel::onTaskGetsRunning(AbsRTTask *t, CPU_BL* cpu) {
        assert (t != NULL); assert (cpu != NULL);
        ECOUT( "t=" << SIMUL.getTime() << ", EMRTK::" << __func__ << "() " << taskname(t) << " on " << cpu->getName() << endl );

        ECOUT( "\t" << cpu->getName() << " had wl: " << cpu->getWorkload() << ", speed: " << cpu->getSpeed() << ", freq: " << cpu->getFrequency() << endl );
        cpu->setWorkload(getEnveloper(t)->getTaskWorkload());
        ECOUT( "\t" << cpu->getName() << " has now wl: " << cpu->getWorkload() << ", speed: " << cpu->getSpeed() << ", freq: " << cpu->getFrequency() << endl << endl );
        assert (cpu->getWorkload() != "");
    }


    // ----------------------------------------------------------- task ends WCET

    /// Callback, when a CBS server ends a task
    void EnergyMRTKernel::onTaskInServerEnd(AbsRTTask* t, CPU_BL* cpu, CBServer* cbs) {
        assert (t != NULL); assert (cpu != NULL); assert(cbs != NULL);
        ECOUT( "\tEMRTK::" << __func__ << "()" << endl );
        
        assert (cbs->isEmpty());
        _queues->onTaskInServerEnd(t, cpu, cbs); // does nothing (was used to save util active)

        onEnd(cbs);
    }

    // task ends WCET
    void EnergyMRTKernel::onEnd(AbsRTTask* t) {
        DBGENTER(_KERNEL_DBG_LEV);

        // only on big-little: update the state of the CPUs island
        CPU_BL* endingCPU = dynamic_cast<CPU_BL*>(getProcessorReadyRunning(t));
        DBGPRINT_6(t->toString(), " has just finished on ", endingCPU->toString(), ". Actual time = [", SIMUL.getTime(), "]");
        ECOUT( ".............................." << endl );
        ECOUT( "\tt=" << SIMUL.getTime() << ", EMRTK::" << __func__ << "(). " << t->toString() << " has just finished on " << endingCPU->getName() << endl );
        _numberEndEvents++;

        _sched->extract(t);
        string state = _sched->toString();
        if (state == "") ECOUT( "\t(External scheduler is empty)" << endl );
        else ECOUT( "\tState of external scheduler: " << endl << "\t\t" << state << endl );

        endingCPU->decreaseTotalNomUtil(getUtilization(t), false); // don't adjust frequency, migrations might happen (thus incresing it again)

        _m_oldExe[t] = endingCPU;
        _m_currExe.erase(endingCPU);
        _m_dispatched.erase(t);
        _queues->onEnd(t, endingCPU); // remove from core queue

        endingCPU->saveUtilization_active(dynamic_cast<CBServer*>(getEnveloper(t)));

        if (_queues->isEmpty(endingCPU) && getRunningTask(endingCPU) == NULL) {
            ECOUT( "t=" << SIMUL.getTime() << ", " << endingCPU->getName() << " core got free" << endl );
            endingCPU->setBusy(false);
            endingCPU->setWorkload("idle");
            assert (endingCPU->getWorkload().compare("idle") == 0);

            if (EMRTK_FORGET_UACTIVE_ON_CORE_FREE) {
                endingCPU->forgetUtilization_active();

                if (EMRTK_MIGRATE_ENABLED && EMRTK_CBS_MIGRATE_AFTER_END) {
                    bool migrationFromOtherIsland = false;
                    if (EMRTK_PLACEMENT_P_EDF_WF)
                    {
                        migrationFromOtherIsland = _performMigration(migrateFromOtherIsland(endingCPU)); // it can make the island freq increase

                        if (!migrationFromOtherIsland)
                            _performMigration(balanceLoad(endingCPU)); // balanceLoad doesn't increase island frequency
                    }
                    else if (EMRTK_PLACEMENT_P_EDF_BF)
                        ECOUT("Migrations disabled with BF" << endl);
                    else if (EMRTK_PLACEMENT_P_EDF_FF)
                        _performMigration(migrateFromRightCoreFF(endingCPU));
                }
                
                endingCPU->getIsland()->adjustFrequency(); // scale down to min freq by updating island _maxUtil

                if (!endingCPU->getIsland()->isBusy()) {
                    ECOUT( "\tt=" << SIMUL.getTime() << ", " << endingCPU->getIsland()->getName() << "'s got free => clock down to min freq" << endl );
                    endingCPU->getIsland()->setOPP(0);
                    // endingCPU->setWorkload("idle");
                    for (const auto& cpu : endingCPU->getIsland()->getProcessors()) {
                        EASSERT (cpu->getWorkload().compare("idle") == 0, cpu); // wl==idle
                        EASSERT (cpu->getOPP() == 0, cpu);
                    }
                }
            }
        }
        else { // core has already some ready tasks
            ECOUT( "\tcore busy, schedule ready task" << endl );
            _queues->schedule(endingCPU);
        }

        ECOUT( endl << endingCPU->pcu() << endl );
        ECOUT( ".............................." << endl );
    }

    // virtual time ends
    void EnergyMRTKernel::onReleasingIdle(CBServer *cbs) {
        ECOUT( "EMRTK::" << __func__ << "()" << endl );
        CBServerCallingEMRTKernel* cbs1 = dynamic_cast<CBServerCallingEMRTKernel*>(cbs);
        CPU_BL* endingCPU = dynamic_cast<CPU_BL*>(cbs1->getJobLastCPU()); // forget U_active
        AbsRTTask* oldTask = getRunningTask(endingCPU);
        vector<AbsRTTask*> readyTasks = getReadyTasks(endingCPU);
        _numberVtimeEvents++;


        if (EMRTK_FORGET_UACTIVE_ON_CORE_FREE && !endingCPU->isBusy())
            endingCPU->forgetUtilization_active();
        else
            endingCPU->forgetUtilization_active(cbs);

        if (!readyTasks.empty()) {
            ECOUT( "\tCore already has some ready tasks. Scheduling one of those" << endl );
            _queues->schedule(endingCPU);
        }
        else if (EMRTK_MIGRATE_ENABLED && EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END) {
            bool migrationFromOtherIsland = false;
                    if (EMRTK_PLACEMENT_P_EDF_WF)
                    {
                        migrationFromOtherIsland = _performMigration(migrateFromOtherIsland(endingCPU)); // it can make the island freq increase

                        if (!migrationFromOtherIsland)
                            _performMigration(balanceLoad(endingCPU)); // balanceLoad doesn't increase island frequency
                    }
                    else if (EMRTK_PLACEMENT_P_EDF_BF)
                        ECOUT("Migrations disabled with BF" << endl);
                    else if (EMRTK_PLACEMENT_P_EDF_FF)
                        _performMigration(migrateFromRightCoreFF(endingCPU));
        }

        endingCPU->getIsland()->adjustFrequency(); // scale down to min freq by updating island _maxUtil

        if (!endingCPU->getIsland()->isBusy()) {
            ECOUT( "\tt=" << SIMUL.getTime() << ", " << endingCPU->getIsland()->getName() << "'s got free => clock down to min freq" << endl );
            endingCPU->getIsland()->setOPP(0);
            endingCPU->setWorkload("idle");
        }
    }


    // ---------------------------------------------------------- migrations

    bool EnergyMRTKernel::_performMigration (MigrationProposal migrationProposal) {
        if (!EMRTK_MIGRATE_ENABLED) { ECOUT( "Migration policy disabled => skip" << endl ); return false; }
        ECOUT( endl << "------------ Migration/balancing ----------" << endl << "\tEMRTK::" << __func__ << "()" << endl );

        if (migrationProposal.task == NULL || migrationProposal.from == NULL || migrationProposal.to == NULL)
            return false;

        ECOUT ( "\tState before migration:" );
        printState(1);

        // move task and adjust cores utils and frequencies
        assert (getEnveloper(migrationProposal.task)->getTaskWorkload() == "bzip2");
        migrationProposal.to->setWorkload(getEnveloper(migrationProposal.task)->getTaskWorkload());
        _queues->onMigrationFinished(migrationProposal.task, migrationProposal.from, migrationProposal.to);  // move task between cores & schedule it
        // getEnveloper(migrationProposal.task)->onMigrated(migrationProposal.to); // update CBS speed and core

        ECOUT ( "\tState after migration:" );
        printState(1);

        ECOUT ( "--------------------" << endl );
        return true;
    }

    EnergyMRTKernel::MigrationProposal EnergyMRTKernel::migrateFromRightCoreFF(CPU_BL* endingCPU)
    {
        /**
         * Migrate onto the ending core the first ready task that fits
         * on the ending core from the cores on the right.
         * 
         * Here cores are ordered as LITTLEs + bigs.
         */

        MigrationProposal migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
        if (!EMRTK_MIGRATE_ENABLED) return migrationProposal;
        ECOUT( "\t\tEMRTK::" << __func__ << "()..." << endl );

        vector<CPU_BL*> processors,
                        littles = getIslandLittle()->getProcessors(),
                        bigs = getIslandBig()->getProcessors();
        processors.reserve(getProcessors().size());
        processors.insert(processors.end(), littles.begin(), littles.end());
        processors.insert(processors.end(), bigs.begin(), bigs.end());

        bool isEndingCoreFound = false;
        for (CPU_BL* cpu : processors)
        {
            if (cpu == endingCPU)
                isEndingCoreFound = true;

            if (isEndingCoreFound)
                for (AbsRTTask* readyTask : getReadyTasks(cpu))
                {
                    CBServerCallingEMRTKernel* cbs = dynamic_cast<CBServerCallingEMRTKernel*>(readyTask);
                    Tick remaining_period = cbs->getDeadline() - SIMUL.getTime();
                    double readyTaskUtil = double(cbs->getRemainingWCETCycles()) / double(remaining_period);
                    if (readyTaskUtil <= getResCapNomCore(endingCPU) 
                    // &&
                        // getUtilization(readyTask) + endingCPU->getUtilizationReadyRunning() + endingCPU->getUtilization_active() <=
                        // endingCPU->getIsland()->getUtilizationLimit())
                    )
                    {
                        migrationProposal.from  = cpu;
                        migrationProposal.task  = readyTask;
                        migrationProposal.to    = endingCPU;
                        return migrationProposal;
                    }
                }
        }

        return migrationProposal;
    }

    // can frequency increase/decrease?
    EnergyMRTKernel::MigrationProposal EnergyMRTKernel::migrateFromOtherIsland (CPU_BL *endingCPU) {
        MigrationProposal migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
        if (!EMRTK_MIGRATE_ENABLED) return migrationProposal;
        ECOUT( "\t\tEMRTK::" << __func__ << "()..." << endl );

        vector<AbsRTTask*> readyTasks;
        if (endingCPU->getIslandType() == IslandType::BIG)
            ECOUT( "\t\tending core is big => skip migrations from big" << endl );
        else { // ending core is little and you can migrate from big island
            if (EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG)   migrationProposal = migrateFromBig_bfeb(endingCPU); // with frequency change
            if (EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG)    migrationProposal = migrateFromOtherIsland_bff(endingCPU, IslandType::BIG);  // no frequency change
        }
        ECOUT( "\t\t...EMRTK::" << __func__ << "()" << endl );
        return migrationProposal;
    }

    EnergyMRTKernel::MigrationProposal EnergyMRTKernel::migrateFromBig_bfeb(CPU_BL* endingCPU) const {
        /**
            Policy: pick the biggest ready job from the most empty big core that fits inside the endingCPU.
                    This way you have more possibilities to accept future jobs in the big core, shouldn't they fit anywhere else.
        */
        MigrationProposal migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
        if (!EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG)
            return migrationProposal;

        vector<UtilCorePair> pairs = getIsland(IslandType::BIG)->getCoresUtilPair()->asVector();
        bool found = false;
        for (auto pair : pairs) { // for each big core
            CPU_BL* emptiestBigCPU = pair.cpu;
            if (found) break;
            
            vector<AbsRTTask*> readyTasks = getReadyTasks(emptiestBigCPU);
            if (readyTasks.empty()) { ECOUT( "\t\t" << emptiestBigCPU->getName() << " has no ready tasks, skip" << endl ); assert (_queues->getReadyTasks(emptiestBigCPU).empty()); continue; }
            
            sort(readyTasks.begin(), readyTasks.end(), [this] (AbsRTTask* t1, AbsRTTask* t2) { return this->getUtilization(t1) > this->getUtilization(t2); });
            AbsRTTask* biggestReady = NULL;
            for (AbsRTTask* t : readyTasks) {
                biggestReady = t;

                if (SIMUL.getTime() == getEnveloper(biggestReady)->getEndEventTime())
                	continue;

                migrationProposal.task      = biggestReady;
                migrationProposal.from      = emptiestBigCPU;
                migrationProposal.to        = endingCPU;
                
                if (isMigrationSafe(migrationProposal) && isMigrationEnergConvenient(migrationProposal)) {
                    found = true;
                    break;
                }
                migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
            }
        }

        return migrationProposal;
    }

    EnergyMRTKernel::MigrationProposal EnergyMRTKernel::migrateFromOtherIsland_bff(CPU_BL* endingCPU, IslandType fromIslandType) const {
        /**
            Migrate biggest from fullest big/little island.
            Policy  case fromIT=big:
                    pick the biggest ready job from the fullest big core that fits inside the endingCPU.
                    Frequencies shall not be changed.
                    This policy allows you to maintain the balancing state of the CPUs utilizations and possibly
                    decrease frequency.
                    
                    case fromIT=little:
                    Not always littles consume less than bigs. Two policies are possible to save energy (board-dep.): keep min freq and idle racing.
                    Min freq => move biggest from fullest little. This way big island keeps its freq and littles' one can decrease.
                    Idle racing => you need to understand if it is better to keep the big idle help the little island.
        */
        MigrationProposal migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
        if (!EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG)
            return migrationProposal;

        bool found = false;
        vector<UtilCorePair> pairs = getIsland(fromIslandType)->getCoresUtilPair()->asVector();
        reverse(pairs.begin(), pairs.end());
        for (auto pair : pairs) { // for each core of the from island
            CPU_BL* fullestCPUOfStartingIsland = pair.cpu; 
            if (found) break;
            
            vector<AbsRTTask*> readyTasks = getReadyTasks(fullestCPUOfStartingIsland);
            if (readyTasks.empty()) { ECOUT( "\t\t" << fullestCPUOfStartingIsland->getName() << " has no ready tasks, skip" << endl ); assert (_queues->getReadyTasks(fullestCPUOfStartingIsland).empty()); continue; }
            
            sort(readyTasks.begin(), readyTasks.end(), [this] (AbsRTTask* t1, AbsRTTask* t2) { return this->getUtilization(t1) > this->getUtilization(t2); });
            AbsRTTask* biggestReady = NULL;
            for (AbsRTTask* t : readyTasks) {
                biggestReady = t;

                if (SIMUL.getTime() == getEnveloper(biggestReady)->getEndEventTime())
                	continue;

                migrationProposal.task      = biggestReady;
                migrationProposal.from      = fullestCPUOfStartingIsland;
                migrationProposal.to        = endingCPU;
                
                if (isMigrationSafe(migrationProposal) && isMigrationEnergConvenient(migrationProposal)) {
                    found = true;
                    break;
                }
                migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
            }

            if (EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG_ONLY_FIRST)
                break;
        }

        return migrationProposal;
    }

    EnergyMRTKernel::MigrationProposal EnergyMRTKernel::balanceLoad(CPU_BL *endingCPU, vector<AbsRTTask*> toBeSkipped) { 
        /**
            Policy: pick a ready job from the fullest big core that fits inside the endingCPU.
            The idea is to pick first the ready tasks with utilization closer to half of the difference
            of the endingCPU and the fullest big core.

            This policy allows you to maintain the balancing state of the CPUs utilizations and possibly
            decrease frequency. Ready tasks are picked that way because moving the biggest ready task from the
            fullest big core would not behave correctly in this situation for example:
            original                        balance1 (better)           balance2 (biggest moved, worse)
            C1: 0.3                         C1: 0.3 0.3                 C1: 0.3 0.5 -> f=800MHz
            C2: 0.1 0.3 0.5 -> f=900MHz     C2: 0.1 0.5 -> f=600MHz     C2: 0.1 0.3
        */
        ECOUT( "\t\tEMRTK::" << __func__ << "(). Balancing load of island: " << endingCPU->getName() << endl );
        MigrationProposal migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
        vector<AbsRTTask*> readyTasks;

        if (!EnergyMRTKernel::EMRTK_BALANCE_ENABLED) {
            ECOUT( "\t\t\tPolicy disabled. skip" << endl );
            return migrationProposal;
        }
        
        bool found = false;
        vector<UtilCorePair> pairs = endingCPU->getIsland()->getCoresUtilPair()->asVector();
        reverse(pairs.begin(), pairs.end());
        for (auto pair : pairs) { // for each big core
            CPU_BL* fullestBigCPU = pair.cpu;
            if (found) break;
            
            vector<AbsRTTask*> readyTasks = sortForBalancing(fullestBigCPU, endingCPU);
            if (readyTasks.empty()) { ECOUT( "\t\t" << fullestBigCPU->getName() << " has no ready tasks, skip" << endl ); assert (_queues->getReadyTasks(fullestBigCPU).empty()); continue; }
            
            AbsRTTask* biggestReady = NULL;
            for (AbsRTTask* t : readyTasks) {
                biggestReady = t;

                if (SIMUL.getTime() == getEnveloper(biggestReady)->getEndEventTime())
                	continue;

                migrationProposal.task      = biggestReady;
                migrationProposal.from      = fullestBigCPU;
                migrationProposal.to        = endingCPU;
                
                if (isMigrationSafe(migrationProposal) && isMigrationEnergConvenient(migrationProposal)) {
                    found = true;
                    break;
                }
                migrationProposal = { .task = NULL, .from = NULL, .to = NULL };
            }

        }

        if (migrationProposal.task == NULL) ECOUT( "\t\t\tNo balancing safe & energy convenient" << endl );
        return migrationProposal;
    }

    vector<AbsRTTask*> EnergyMRTKernel::sortForBalancing(CPU_BL* from, CPU_BL* to) {
        /**
          * The aim is that the two cores have core utilization as much simular as
          * possible. Therefore, ready tasks of pulled core are ordered wrt their
          * vicinity to half of the difference of the two cores utilizations.
          */
        double utilFrom  = from->getUtilizationReadyRunning();
        double utilTo    = to->getUtilizationReadyRunning();
        double utilDiff  = fabs(utilFrom - utilTo) / 2.0;

        struct task_utilDiff { AbsRTTask* task; double taskUtilDiff; };
        vector<struct task_utilDiff> tableTask_utilDiff;
        vector<AbsRTTask*> readyTasks = getReadyTasks(from);
        for (AbsRTTask* t : readyTasks) {
            double diff = fabs(utilDiff - getUtilization(t));
            tableTask_utilDiff.push_back({ t, diff });
        }

        sort(tableTask_utilDiff.begin(), tableTask_utilDiff.end(),
            [] (struct task_utilDiff const& e1, struct task_utilDiff const& e2) { return e1.taskUtilDiff > e2.taskUtilDiff; });

        vector<AbsRTTask*> readyTasksOrdered;
        for (struct task_utilDiff& t : tableTask_utilDiff) readyTasksOrdered.push_back(t.task);
        return readyTasksOrdered;
    }

    bool EnergyMRTKernel::isMigrationSafe(const MigrationProposal mp, bool frequencyCanChange /*= true*/) const {
        /**
            It's safe to migrate the proposed task if on the ending core it holds:
                WCET_scaled / (abs DL - now) <= (1 - U_ending_core) 

            Frequency increases can be taken into account or not based on frequencyCanChange.
        **/

        assert (mp.task != NULL); assert (mp.from != NULL); assert (mp.to != NULL);
        CPU_BL *endingCPU = mp.to;
        Island_BL* islandFrom = mp.from->getIsland();
        Island_BL* islandTo   = mp.to->getIsland();
        CBServerCallingEMRTKernel *task_m = dynamic_cast<CBServerCallingEMRTKernel*> (mp.task);
        
        string starting_wl = endingCPU->getWorkload();
        endingCPU->setWorkload(task_m->getTaskWorkload());
        
        double activeUtilTo_nom = endingCPU->getUtilization_active(); // unscaled, already / REFERENCE_SPEED
        double utilCoreTo_nom = endingCPU->getUtilizationReadyRunning() + activeUtilTo_nom;
        double utilJob_nom = double(task_m->getWCET(1.0)) / double(task_m->getDeadline() - SIMUL.getTime()); // WCET / remaining period
        double utilCoreToWithMigr_nom = utilCoreTo_nom + utilJob_nom;
        double newFreqTo = islandTo->getFrequencyFromGraph(utilCoreToWithMigr_nom);
        assert (task_m->getDeadline() - SIMUL.getTime() > 0);
        
        if (newFreqTo != endingCPU->getFrequency() && !frequencyCanChange) {
            ECOUT( "\t\tNot safe to migrate " << task_m->toString() << " into " << endingCPU->toString() << " keeping old freq" << endl );
            endingCPU->setWorkload(starting_wl);
            return false;
        }
        if (newFreqTo == -1) {
            ECOUT( "\t\tNot safe to migrate " << task_m->toString() << " into " << endingCPU->toString() << ": you'd go above LITTLE max freq" << endl );
            endingCPU->setWorkload(starting_wl);
            return false;
        }

        bool safe = ( ceil(task_m->getWCET(1.0)) <= (islandTo->getUtilizationLimit() - utilCoreTo_nom) * double(task_m->getDeadline() - SIMUL.getTime()) ); // remaining utilization of migrated task > 1 - U_core (remaining core util)

        if (!safe)
            ECOUT( "\t\tNot safe to migrate " << task_m->toString() << " into " << endingCPU->toString() << ", t=" << SIMUL.getTime() << endl );
        else {
            ECOUT( "\t\tMigration safe " << task_m->toString() << getUtilization(task_m) << ": " << mp.from->toString() << " -> " << endingCPU->toString() << " with new freq " << newFreqTo << " and new total util " << utilCoreToWithMigr_nom << ", t=" << SIMUL.getTime() << endl );
            ECOUT ( "\t\t\t" << mp.to->getName() << " has now util tot=" << utilCoreTo_nom << " and after migration " << utilCoreToWithMigr_nom << endl );
        }

        endingCPU->setWorkload(starting_wl);
        return safe;
    }

    bool EnergyMRTKernel::isMigrationEnergConvenient(const MigrationProposal mp) const {
        /**
          * Check must be on the island because frequency change is on the whole island.
          * Check considers a possible frequency change.
          *
          * This algorithm is inspired by the one in EAS documentaton, Linux 5.1.
          * Note: this alg. is wrong when tasks have different instructions, with different wl (power is the sum of each instruction)
          */
        ECOUT( "\tEMRTK::" << __func__ << "() " << endl );
        Island_BL* islandFrom     = mp.from->getIsland();
        Island_BL* islandTo       = mp.to->getIsland();
        Island_BL* littleIsland   = _islands[IslandType::LITTLE];
        Island_BL* bigIsland      = _islands[IslandType::BIG];
        vector<CPU_BL*> processors = getProcessors();

        double totalConsumption_cur = 0.0, totalConsumption_new = 0.0; // energy cons. before and after migration

        // find current islands energy consumption
        for (CPU_BL* c : processors) {
          double power_cur = c->getCurrentPowerConsumption();
          totalConsumption_cur += ( c->getUtilization_active() + c->getUtilizationReadyRunning() ) * power_cur;
        }

        // find islands new frequency. First you need the new max core
        double newUtil[processors.size()] = { 0.0 }; // little; big cores
        int max_little_i = 0, max_big_i = littleIsland->getProcessors().size(), i = 0;
        for (CPU_BL* c : processors) {
          double curUtil = ( c->getUtilization_active() + c->getUtilizationReadyRunning() );
          if (c == mp.from)
            newUtil[i] = curUtil - getUtilization(mp.task);
          else if (c == mp.to) {
            double u_infl = mp.task->getWCET(1.0) / double(mp.task->getDeadline() - SIMUL.getTime());
            newUtil[i] = curUtil + u_infl;
          }

          if (c->isBig()  && newUtil[i] > newUtil[max_big_i])
            max_big_i = i;
          else if (!c->isBig() && newUtil[i] > newUtil[max_little_i])
            max_little_i = i;
          i++;
        }

        double newPow_big    = bigIsland->getPowerConsFromGraph(newUtil[max_big_i]);
        double newPow_little = littleIsland->getPowerConsFromGraph(newUtil[max_little_i]);

        i = 0;
        for (CPU_BL* c : processors) {
          double newPow = (c->isBig() ? newPow_big : newPow_little);
          totalConsumption_new += newUtil[i] * newPow;
          i++;
        }

        // this function considers a frequency increasing
        bool   isConvenient       = (totalConsumption_new < totalConsumption_cur);

        ECOUT( "\t\tcur energy: " << totalConsumption_cur << " VS new energy: " << totalConsumption_new << endl );
        ECOUT( "\t\tisConvenient to migrate task: " << (isConvenient ? "yes" : "no") << endl );
        return isConvenient;
    }

    double EnergyMRTKernel::getPowerDelta(MigrationProposal mp, string where) const {
        ECOUT( "\tEMRTK::" << __func__ << "()" << endl );
        assert (mp.task != NULL); assert (mp.from != NULL); assert (mp.to != NULL);
        
        CPU_BL *endingCPU          = mp.to;
        Island_BL* islandFrom      = mp.from->getIsland();
        Island_BL* islandTo        = mp.to->getIsland();
        double activeUtilTo_nom    = mp.to->getUtilization_active() * mp.to->getSpeed() / CPU_BL::REFERENCE_SPEED;
        double oldNomUtilTo_nom    = mp.to->getUtilizationReadyRunning() + activeUtilTo_nom;
        double newNomUtilTo_nom    = oldNomUtilTo_nom   + getUtilization(mp.task);

        double activeUtilFrom_nom  = mp.from->getUtilization_active() * mp.from->getSpeed() / CPU_BL::REFERENCE_SPEED;
        double oldNomUtilFrom_nom  = mp.from->getUtilizationReadyRunning() + activeUtilFrom_nom;
        double newNomUtilFrom_nom  = oldNomUtilFrom_nom - getUtilization(mp.task);

        double powerBeforeTo       = islandTo->getPowerConsFromGraph(oldNomUtilTo_nom);
        double powerAfterTo        = islandTo->getPowerConsFromGraph(newNomUtilTo_nom);

        double powerBeforeFrom     = islandFrom->getPowerConsFromGraph(oldNomUtilFrom_nom);
        double powerAfterFrom      = islandFrom->getPowerConsFromGraph(newNomUtilFrom_nom);
        
        double delta = powerAfterFrom - powerBeforeFrom;
        if (where == "to")
            delta = powerAfterTo - powerBeforeTo;

        return delta;
    }

    void EnergyMRTKernel::onRound(AbsRTTask *finishingTask) {
        ECOUT( "t = " << SIMUL.getTime() << " " << __func__ << " for finishingTask = " << taskname(finishingTask) << endl );
        finishingTask->deschedule();
        _queues->onRound(finishingTask, getProcessorReadyRunning(finishingTask));
    }

    // ---------------------------------------------------------- CBServer management

    CBServerCallingEMRTKernel* EnergyMRTKernel::addTaskAndEnvelope(AbsRTTask *t, const string &param) { 
        CBServerCallingEMRTKernel *serv = dynamic_cast<CBServerCallingEMRTKernel*>(t);

        if (serv == NULL) { // periodic task
            double wcet = t->getWCET(1.0);
            Tick WCET_cycles = Tick::ceil(wcet);
            Tick extra = 0;
            double scalingFactor = 0.0;
            if (EMRTK_ENLARGE_WCET) {
                if (EMRTK_ENLARGE_WCET_10_PERC) { extra = Tick::ceil(0.1 * wcet); scalingFactor = 0.1; } // for Ewili2019 it was this
                else if (EMRTK_ENLARGE_WCET_UNIF) {
                    // use the task WCET as CBS budget and reduce task WCET by [0.6-0.9]
                    Task* tt = dynamic_cast<Task*>(t);
                    UniformVar r(0.6, 0.9);
		    r.init(tt->getIndex());
                    double rand = r.get();

                    double newTaskWCET = ceil(wcet * rand);
                    char instr[60] = "";
                    scalingFactor = rand;
                    tt->discardInstrs();
                    sprintf(instr, "fixed(%f, %s);", newTaskWCET, "bzip2");
                    tt->insertCode(instr);
                    
		    if ( newTaskWCET == double(WCET_cycles) )
                        extra  = 5;
                }
            } 
            serv = new CBServerCallingEMRTKernel(WCET_cycles + extra, t->getDeadline(), t->getDeadline(), "hard", "CBS(" + t->toString() + ")", "FIFOSched");
            serv->addTask(*t);
		    serv->setScalingFactor(scalingFactor);

	    cout << serv->toString() << endl;

            addTask(*serv, param);

            assert ( _envelopes.find(t) == _envelopes.end() );
            _envelopes[t] = serv;
        }

        return serv;
    }

    void EnergyMRTKernel::onExecutingReleasing(CBServer* cbs) {
        ECOUT( "EMRTK::" << __func__ << "()" << endl );
        CPU *cpu = getOldProcessor(cbs);

        // for some reason, here task has wl idle, wrongly (should be kept until the end of this function). reset:
        // string original_wl = cpu->getWorkload();
        // cpu->setWorkload("bzip2");
        // ECOUT( "\t" << cpu->getName() << " has now wl: " << cpu->getWorkload() << ", speed: " << cpu->getSpeed() << endl );

        _queues->onExecutingReleasing(cpu, cbs); // does nothing relevant

        if (EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED) {
            /**
            Schedule ready task on core
            */
            assert (cbs->getStatus() == ServerStatus::RELEASING);
            ECOUT( "CBS enveloping periodic tasks enabled => schedule ready on " << cpu->getName() << endl );
            _queues->schedule(cpu);
        }

        // cpu->setWorkload(original_wl);
    }

    // ----------------------------------------------------------- choosing cores for tasks

    void EnergyMRTKernel::leaveLittle3(AbsRTTask* t, vector<struct ConsumptionTable> iDeltaPows, CPU_BL*& chosenCPU) {
        /**
         * Policy of leaving a little core free for big-WCET tasks, which otherwise would be scheduled on
         * big cores, thus increasing power consumption.
         * Mechanism: if you want to put a task on little 3, but it also fits in another little core,
         * put it in the other little. Otherwise (if it only fits on little 3 and in bigs, choose little 3).
         */

        ECOUT( __func__ << "():" << endl );
        if (!EMRTK_LEAVE_LITTLE3_ENABLED) { ECOUT( "\tPolicy deactivated. Skip"<<endl ); return; }
        if (chosenCPU->getName().find("LITTLE_3") == string::npos || chosenCPU->getIslandType() == IslandType::BIG) {
            ECOUT( "chosenCPU in big island or is not little_3 => skip" << endl );
            return;
        }

        if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED && dynamic_cast<PeriodicTask*>(t))
            t = getEnveloper(t);

        bool fitsInOtherCore = true; // if it only fits on little 3 and in bigs

        for (int i = 0; i < iDeltaPows.size(); i++) {
            ECOUT( iDeltaPows[i].cons << " " << iDeltaPows[i].cpu->toString() << endl );
            if (iDeltaPows[i].cpu->getIslandType() == IslandType::LITTLE && iDeltaPows[i].cpu->getName().find("LITTLE_3") == string::npos) {
                chosenCPU = iDeltaPows[i].cpu;
                chosenCPU->setOPP(iDeltaPows[i].opp);
                // ECOUT( "Changing to " << iDeltaPows[i].cpu->toString() << " - chosenCPU=" << chosenCPU->toString() << endl );
                fitsInOtherCore = false;
                break;
            }
        }

        if (fitsInOtherCore) {
            ECOUT( "Task only fits on little 3 and in bigs => stay in LITTLE_3, CPU not changed" << endl );
        }
    }

    void EnergyMRTKernel::dispatch(AbsRTTask *t, CPU *p) {
        ECOUT( "t=" << SIMUL.getTime() << ", EMRTK::" << __func__ << "(). Confirmed dispatch " << t->toString() << " -> " << p->getName() << endl );
        CPU_BL* pp = dynamic_cast<CPU_BL*>(p);

        _queues->insertTask(t, pp);
        pp->setBusy(true);        
    }

    /* Decide a CPU for each ready task */
    void EnergyMRTKernel::dispatch() {
        DBGENTER(_KERNEL_DBG_LEV);

        int num_newtasks    = 0; // # "new" tasks in the ready queue
        int i               = 0;
        
        while (_sched->getTaskN(num_newtasks) != NULL)
            num_newtasks++;

        // _sched->print();
        DBGPRINT_2("New tasks: ", num_newtasks);
        if (num_newtasks == 0) return; // nothing to do

        i = 0;
        std::vector<CPU_BL*> cpus = getProcessors();
        do {
            AbsRTTask *t = dynamic_cast<AbsRTTask*>(_sched->getTaskN(i++));
            if (t == NULL) break;

            // for testing
            #ifdef DEBUG
            if (manageForcedDispatch(t) || manageDiscartedTask(t)) {
                num_newtasks--;
                continue;
            }
            #endif

            if (_queues->isInAnyQueue(t)) {
                // dispatch() is called even before onEndMultiDispatch() finishes and thus tasks seem
                // not to be dispatching (i.e., assigned to a processor)
                ECOUT( "\tTask has already been dispatched, but dispatching is not complete => skip (you\'ll still see desched&sched evt, to trace tasks)" << endl );
                continue;
            }

            if (getProcessorReadyRunning(t) != NULL) { // e.g., task ends => migrateInto() => dispatch()
                ECOUT( "\tTask is running on a CPU already => skip" << endl );
                continue;
            }

            // otherwise scale up CPUs frequency
            ECOUT( endl << "\t------------\n\tcurrent situation t=" << SIMUL.getTime() << ":\n\t" << _queues->toString() << "\t------------" << endl );
            ECOUT( "t=" << SIMUL.getTime() << ". Dealing with task " << t->toString() << ", u_nom=" << getUtilization(t) << endl );

            /// The very hearth of the algorithm
            placeTask(t);
            
            _sched->extract(t);
            num_newtasks--;

            // if you get here, task is not schedulable in real-time
        } while (num_newtasks > 0);

        ECOUT( "" << endl );
        for (CPU_BL *c : getProcessors()) {
            _queues->schedule(c);
        }
        ECOUT( "" << endl );
    }

    void EnergyMRTKernel::placeTask(AbsRTTask* t, vector<CPU_BL*> toBeExcluded)
    {
        if (EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_WF)
            placeTask_WF(t, toBeExcluded);
        else if (EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_BF)
            placeTask_BF(t, toBeExcluded);
        else if (EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_FF)
            placeTask_FF(t, toBeExcluded);
        else
        {
            printf("No valid placement strategy given");
            abort();
        }
    }

    void EnergyMRTKernel::placeTask_WF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded)
    {
        struct timespec beg_ts, end_ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &beg_ts);
        bool hasBeenDispatched = false;
        assert (dynamic_cast<CBServerCallingEMRTKernel*>(t)->getStatus() == READY);

        Island_BL* bigIsland                        = _islands[IslandType::BIG];
        Island_BL* littleIsland                     = _islands[IslandType::LITTLE];
        const double utilLimitLittle                = littleIsland->getUtilizationLimit();

        CPU_BL* cpuWithLowestUtilLittle = NULL, * cpuWithLowestUtilBig = NULL;
        double lowestUtilLittle = 999.0, lowestUtilBig = 999.0;
        string originalWlBig, originalWlLittle;

        UtilCorePair lowestUtilzedLittle            = littleIsland->getCoreWithLowestUtilization();
        lowestUtilLittle                            = lowestUtilzedLittle.util;
        cpuWithLowestUtilLittle                     = lowestUtilzedLittle.cpu;



        UtilCorePair lowestUtilzedBig               = bigIsland->getCoreWithLowestUtilization();
        lowestUtilBig                               = lowestUtilzedBig.util;
        cpuWithLowestUtilBig                        = lowestUtilzedBig.cpu;


        double  newUtilLittle                       = lowestUtilLittle + getUtilization(t);

        // task not schedulable
        if (toBeExcluded.size() == getProcessors().size() || newUtilLittle > CPU_BL::REFERENCE_SPEED) {
            cerr << "t=" << SIMUL.getTime() << ". Cannot schedule " << t->toString() << " anywhere" << endl << endl;
            ECOUT( "t=" << SIMUL.getTime() << ". Cannot schedule " << t->toString() << " anywhere" << endl << endl );
            if (EMRTK_ABORT_ON_NOT_SCHEDULABLE) {
                printState(true, true);
                abort();
            }
        }

        // normal cases: choose a core
        if (newUtilLittle > utilLimitLittle) { // u_limit_little = 0.3447265625
            ECOUT( __func__ << "(), newUtilLittle > utilLimitLittle (i.e, task does not fit in little island at all. So choose big island, full stop)" << endl );
            // new task is for big island
            if (isAdmissible(t, cpuWithLowestUtilBig)) {
                clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
                hasBeenDispatched = true;
                dispatch(t, cpuWithLowestUtilBig);
            }
            else {
                ROUTINE_ON_NON_SCHEDULABLE(t, "1");
            }
        }
        else if (newUtilLittle <= utilLimitLittle ) {
            ECOUT( __func__ << "(), newUtilLittle <= utilLimitLittle (i.e., task fits in little with min util)" << endl );
            // new task fits inside a little core
            double maxUtilLittle = littleIsland->getMaxUtilization().begin()->first;
            if (newUtilLittle <= maxUtilLittle) {
                ECOUT( __func__ << "(), newUtilLittle <= maxUtilLittle (i.e., task doesn't make LITTLE island freq increase)" );
                // if new job doesn't make LITTLE island freq increase, put it there
                if (isAdmissible(t, cpuWithLowestUtilLittle)) {
                    clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
                    hasBeenDispatched = true;
                    dispatch(t, cpuWithLowestUtilLittle);
                }
                else {
                    ROUTINE_ON_NON_SCHEDULABLE(t, "2");
                }
            }
            else if ( maxUtilLittle < newUtilLittle && newUtilLittle < utilLimitLittle ) {
                // new task is the new one with max utilization
                double newFreqLittle = littleIsland->getFrequencyFromGraph(newUtilLittle);
                double curFreqLittle = littleIsland->getFrequency();
                if (newFreqLittle == curFreqLittle) {
                    ECOUT( __func__ << "(), newFreqLittle == curFreqLittle (i.e., new task makes the little core get the new one with max util, and freq does not increase)" << endl );
                    if (isAdmissible(t, cpuWithLowestUtilLittle)) {
                        clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
                        hasBeenDispatched = true;
                        dispatch(t, cpuWithLowestUtilLittle);
                    }
                    else {
                        ROUTINE_ON_NON_SCHEDULABLE(t, "3");
                    }
                }
                else { // little changes frequency; big can or not change it. what's better?
                    ECOUT( __func__ << "(), new task makes the little core get the new one with max util, and freq does not increase. Better to choose big?" << endl );
                    // let's say tau_new goes on little
                    double newPowLittle = littleIsland->getPowerConsFromGraph(newUtilLittle);
                    double newEnergyLittle = newUtilLittle * newPowLittle;

                    for (CPU_BL* c : littleIsland->getProcessors())
                        if (c != cpuWithLowestUtilLittle)
                            newEnergyLittle += c->getUtilizationReadyRunning() * newPowLittle;

                    // let's say tau_new goes on big
                    double newUtilBig   = lowestUtilBig + getUtilization(t);
                    double newFreqBig   = bigIsland->getFrequencyFromGraph(newUtilBig);

                    double newPowBig    = bigIsland->getPowerConsFromGraph(newUtilBig);
                    double newEnergyBig = newUtilBig * newPowBig;

                    for (CPU_BL* c : bigIsland->getProcessors())
                        if (c != cpuWithLowestUtilBig)
                            newEnergyBig    += c->getUtilizationReadyRunning() * newPowBig; // core energy with new pow cons
                    

                    CPU_BL* chosenCPU = NULL;
                    if (newEnergyLittle <= newEnergyBig) {
                        chosenCPU = cpuWithLowestUtilLittle;
                    }
                    else {
                        chosenCPU = cpuWithLowestUtilBig;
                    }
                    if (EnergyMRTKernel::EMRTK_PLACEMENT_LITTLE_FIRST) // policy little cores first, even if non-sense
                        chosenCPU = cpuWithLowestUtilLittle;
                    if (isAdmissible(t, chosenCPU)) {
                        clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
                        hasBeenDispatched = true;
                        dispatch(t, chosenCPU);
                    }
                    else {
                        ROUTINE_ON_NON_SCHEDULABLE(t, "4");
                    }
                }
            }
            else {
                ECOUT( __func__ << "(), else" << endl );
                if (isAdmissible(t, cpuWithLowestUtilBig)) {
                    clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
                    hasBeenDispatched = true;
                    dispatch(t, cpuWithLowestUtilBig);
                }
                else {
                    ROUTINE_ON_NON_SCHEDULABLE(t, "5");
                }
            }

        }

        // Calculating total time taken by the very hearth of the placing algorithm
        if (EMRTK_PRINT_PLACEMENT_TIME) {
            unsigned long time_taken_us = (end_ts.tv_sec - beg_ts.tv_sec) * 1000000 + (end_ts.tv_nsec - beg_ts.tv_nsec) / 1000;
            cout << "t=" << SIMUL.getTime() << ", placement algorithm - " << fixed << time_taken_us << setprecision(6) << " us" << endl;
        }

        if (EMRTK_ABORT_ON_NOT_SCHEDULABLE)
            assert (hasBeenDispatched);
    }

    void EnergyMRTKernel::placeTask_BF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded)
    {
        /**
         * BF -> new task t dispatched to core with minimum residual capacity
         * 
         * BestFit tends to use the cores already used if there is enough space
         * and it takes the core with minimum residual capacity. If the task t
         * does not fit in any of the cores already in use, then t is dispatched
         * onto an empty core.
         * 
         * When computing the core residual capacity, the island current OPPs
         * are taken into account. If the current OPP is not sufficient, the
         * next OPPs are considered.
         * 
         * Migrations are disabled with BF. BF should be worse than WF in terms of energy.
         */

        // LITTLE or big island for t?
        vector<CPU_BL*> processors;
        if (getUtilization(t) <= getIslandLittle()->getUtilizationLimit())
            processors = getIslandLittle()->getProcessors();
        else
            processors = getIslandBig()->getProcessors();

        map<CPU_BL*, string> oldWls;
        for(const auto& c : getProcessors())
        {
            oldWls[c] = c->getWorkload();
            c->setWorkload("bzip2");
        }

        ECOUT("Considering " << processors.at(0)->getIsland()->getName() << " without increasing OPP" << endl);
        bool    hasBeenDispatched      =   false,
                canIncreaseOPP         =   false;
        while (!hasBeenDispatched) 
        {
            // we check all cores with tasks of the island sorted by
            // residual cap (e.g., 0.2, 0.3, 0.5).
            vector<CPU_BL*> cpuMinResCaps; // not nominal
            for (const auto& c : processors)
                if (!_queues->isEmpty(c))
                    cpuMinResCaps.push_back(c);

            sort(cpuMinResCaps.begin(), cpuMinResCaps.end(),
                [this, canIncreaseOPP] (CPU_BL* e1, CPU_BL* e2) 
                { return  getResCapNomCore(e1, canIncreaseOPP) < getResCapNomCore(e2, canIncreaseOPP); });

            for (const auto& cc : cpuMinResCaps)
                ECOUT("P-EDF-BL, Rem Cap for " 
                        << cc->toString() << "  --->   " << getResCapNomCore(cc, canIncreaseOPP) 
                        << " - " << (canIncreaseOPP ? "" : " not") 
                        << " considering to increse OPP." << endl);

            for (const auto& cpuMinResCap : cpuMinResCaps)
            {
                double newUtilCoreMinRes = getUtilization(t) + cpuMinResCap->getUtilizationReadyRunning() + cpuMinResCap->getUtilization_active();
                // task fits onto the core with min res cap with any OPP?
                if (getUtilization(t) <= getResCapNomCore(cpuMinResCap, canIncreaseOPP) &&
                    newUtilCoreMinRes <= cpuMinResCap->getIsland()->getUtilizationLimit())
                {
                    dispatch(t, cpuMinResCap);
                    hasBeenDispatched = true;
                    cpuMinResCap->getIsland()->adjustFrequency();
                    break; // innermost loop
                }
            }

            // if task does not fit in any core of LITTLE island
            // with the cur OPP, try increasing it;
            // if with any OPP t does not fit in the LITTLE island, try big island
            if (!hasBeenDispatched)
            {
                if (!canIncreaseOPP) // no OPP increase until now
                {
                    canIncreaseOPP = true; // try increasing island OPP
                    ECOUT("Considering OPP increase on " << processors.at(0)->getIsland()->getName() << endl);
                    continue;
                }
                else
                {
                    // just tried increasing OPP with no result.
                    // there is a free core?
                    for (const auto& c : processors)
                        if (_queues->isEmpty(c))
                        {
                            assert (getUtilization(t) <= c->getIsland()->getUtilizationLimit());
                            dispatch(t, c);
                            hasBeenDispatched = true;
                            c->getIsland()->adjustFrequency();
                            break; // innermost loop
                        }
                }
                
                // try big island without increasing OPP
                if (processors.at(0)->getIslandType() == LITTLE)
                {
                    processors = getIslandBig()->getProcessors();
                    canIncreaseOPP = false;
                    ECOUT("LITTLE island not enough. Considering big island without increasing OPP" << endl);
                    continue;
                }

                break; // innermost loop
            }
        }

        for(const auto& c : getProcessors())
            c->setWorkload(oldWls[c]);

        if (EMRTK_ABORT_ON_NOT_SCHEDULABLE)
            assert (hasBeenDispatched);
    }

    void EnergyMRTKernel::placeTask_FF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded)
    {
        /**
         * FF -> the new task t is dispatched onto the first core on which it fits.
         *       it accumulates the tasks on the left.
         * 
         * Here, the islands are ordered LITTLE + big.
         * 
         * Migrations are not disabled with FF. 
         * FF shall be worse than WF in term of energy.
         */

        bool hasBeenDispatched = false;
        vector<CPU_BL*> processors,
                        littles = getIslandLittle()->getProcessors(),
                        bigs = getIslandBig()->getProcessors();
        processors.reserve(getProcessors().size());
        processors.insert(processors.end(), littles.begin(), littles.end());
        processors.insert(processors.end(), bigs.begin(), bigs.end());

        for (CPU_BL* cpu : processors)
        {
            double coreResCapNom = getResCapNomCore(cpu);

            if (getUtilization(t) <= coreResCapNom)
            {
                dispatch(t, cpu);
                hasBeenDispatched = true;
                break;
            }
        }

        if (EMRTK_ABORT_ON_NOT_SCHEDULABLE)
            assert (hasBeenDispatched);
    }

    // ------------------------- GRUB-PA Linux 5.3.11

    /**
     * GRUB-PA would implement both the bandwidth
     * greedy reclaimation + energy awareness.
     * We implement only the energy awareness part,
     * where a task tau_new arrives and
     * 1. if there is a free core, it is dispatched there
     * as a running task
     * 2. if all core have a ready/running task, tau_new
     * is dispatched into the later-deadline core. It
     * preempts tau_old, tau_new gets running, and tau_old
     * is maintained on the same core as 
     * before as a ready task.
     * 
     * After each push (i.e., a core receives a task), 
     * we take for each island the core with 
     * max active utilization
     * (i.e., utilization of running + utilization 
     * of ready tasks)
     * and set the OPP based on it. Notice that, compared to
     * G-EDF, GRUB-PA does use the queues for the cores, and
     * thus for each core we remember the ready and the running
     * tasks, along with their utilizations.
     * 
     * GRUB-PA mainline does not make any distinction between
     * the 2 islands. Thus, when a task tau_new 
     * with nominal utilization 
     * bigger than the max little island speed arrives,
     * and that core is free or with later 
     * deadline, tau_new is
     * dispatched onto that core even though
     * we know a-priori that it will miss its deadline.
     * Also, there is no admission test when dispatching tau_new.
     * This means on big.LITTLE that the core of an island with
     * maximum utilization might have utilization greater
     * than its maximum speed, which corresponds to no frequency,
     * and in this case we just
     * select the island maximum OPP.
     */

    MRTKernel_Linux5_3_11_GRUB_PA::MRTKernel_Linux5_3_11_GRUB_PA(vector<Scheduler*> &qs, Scheduler *s, Island_BL* big, Island_BL* little, const string &name)
        : EnergyMRTKernel(qs, s, big, little, name)
    {
    }

    /* Decide a CPU for each ready task */
    void MRTKernel_Linux5_3_11_GRUB_PA::dispatch()
    {
        DBGENTER(_KERNEL_DBG_LEV);

        int num_newtasks = 0; // # "new" tasks in the ready queue
        int i = 0;

        while (_sched->getTaskN(num_newtasks) != NULL)
            num_newtasks++;

        // _sched->print();
        DBGPRINT_2("New tasks: ", num_newtasks);
        if (num_newtasks == 0)
            return; // nothing to do

        i = 0;
        std::vector<CPU_BL *> cpus = getProcessors();
        do
        {
            AbsRTTask *t = dynamic_cast<AbsRTTask *>(_sched->getTaskN(i++));
            if (t == NULL)
                break;

            // for testing
            #ifdef DEBUG
            if (manageForcedDispatch(t) || manageDiscartedTask(t))
            {
                num_newtasks--;
                continue;
            }
            #endif

            if (_queues->isInAnyQueue(t))
            {
                // dispatch() is called even before onEndMultiDispatch() finishes and thus tasks seem
                // not to be dispatching (i.e., assigned to a processor)
                ECOUT("\tTask has already been dispatched, but dispatching is not complete => skip (you\'ll still see desched&sched evt, to trace tasks)" << endl);
                continue;
            }

            if (getProcessorReadyRunning(t) != NULL)
            { // e.g., task ends => migrateInto() => dispatch()
                ECOUT("\tTask is running on a CPU already => skip" << endl);
                continue;
            }

            // otherwise scale up CPUs frequency
            ECOUT(endl
                  << "\t------------\n\tcurrent situation t=" << SIMUL.getTime() << ":\n\t" << _queues->toString() << "\t------------" << endl);
            ECOUT("t=" << SIMUL.getTime() << ". Dealing with task " << t->toString() << ", u_nom=" << getUtilization(t) << endl);

            /// The very hearth of the algorithm
            placeTask(t);

            _sched->extract(t);
            num_newtasks--;

            // if you get here, task is not schedulable in real-time
        } while (num_newtasks > 0);

        ECOUT("" << endl);
        for (CPU_BL *c : getProcessors())
            _queues->schedule(c);
        ECOUT("" << endl);
    }

    void MRTKernel_Linux5_3_11_GRUB_PA::placeTask(AbsRTTask *taskinside, vector<CPU_BL *> toBeExcluded)
    {
        AbsRTTask* t = getEnveloper(taskinside); // CBS server

        vector<CPU_BL*> freeCPUs;
        for (const auto& cpu : getProcessors())
            if (_queues->isEmpty(cpu))
               freeCPUs.push_back(cpu);

        if (freeCPUs.size() > 0)
        {
            EnergyMRTKernel::dispatch(t, freeCPUs.at(0));
            return;
        }

        CPU_BL* laterCPU = getIslandBig()->getProcessors().at(0);
        for (const auto& cpu : getProcessors())
        {
            CBServerCallingEMRTKernel *hp_cpu = getEnveloper(_queues->getHighestPriority(cpu));

            // handling corner-case
            if (hp_cpu->getEndEventTime() == SIMUL.getTime())
                cerr << "Detected corner case at " << __FILE__ << " -> " << __func__ << endl;

            // heart of the for
            if (hp_cpu->getDeadline() > _queues->getHighestPriority(laterCPU)->getDeadline())
                laterCPU = cpu;

        }
        assert (laterCPU != NULL);
        EnergyMRTKernel::dispatch(t, laterCPU);
    }

    // task ends WCET
    void MRTKernel_Linux5_3_11_GRUB_PA::onEnd(AbsRTTask *t)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        // only on big-little: update the state of the CPUs island
        CPU_BL *endingCPU = dynamic_cast<CPU_BL *>(getProcessorReadyRunning(t));
        DBGPRINT_6(t->toString(), " has just finished on ", endingCPU->toString(), ". Actual time = [", SIMUL.getTime(), "]");
        ECOUT(".............................." << endl);
        ECOUT("\tt="    << SIMUL.getTime() 
                        << ", ML5311GRUB-PA::" << __func__ << "(). " 
                        << t->toString() << " has just finished on " 
                        << endingCPU->getName() 
                        << endl);
        _numberEndEvents++;
        // assert (endingCPU->getWorkload().compare("idle") != 0); // wl != idle

        _sched->extract(t);
        string state = _sched->toString();
        if (state == "")
            ECOUT("\t(External scheduler is empty)" << endl);
        else
            ECOUT("\tState of external scheduler: " << endl
                                                    << "\t\t" << state 
                                                    << endl);

        endingCPU->decreaseTotalNomUtil(getUtilization(t), false); // don't adjust frequency, migrations might happen (thus incresing it again)

        _m_oldExe[t] = endingCPU;
        _m_currExe.erase(endingCPU);
        _m_dispatched.erase(t);
        _queues->onEnd(t, endingCPU); // remove from core queue

        endingCPU->saveUtilization_active(dynamic_cast<CBServer *>(getEnveloper(t)));

        if (_queues->isEmpty(endingCPU) && getRunningTask(endingCPU) == NULL)
        {
            ECOUT("t=" << SIMUL.getTime() << ", " << endingCPU->getName() << " core got free" << endl);
            endingCPU->setBusy(false);
            endingCPU->setWorkload("idle");

            // take the ready task with closest deadline
            // on the other cores and pull it.
            AbsRTTask* closetDeadlineTask = NULL;
            CPU_BL* itsCPU = NULL;
            for (const auto& cpu : getProcessors())
                for (const auto& t : _queues->getReadyTasks(cpu))
                {
                    if (dynamic_cast<CBServerCallingEMRTKernel*>(t)->getEndEventTime() == SIMUL.getTime())
                        continue;
                        
                    if (closetDeadlineTask == NULL)
                    {
                        closetDeadlineTask = t;
                        itsCPU = cpu;
                    }
                    else if (t->getDeadline() < closetDeadlineTask->getDeadline())
                    {
                        closetDeadlineTask = t;
                        itsCPU = cpu;
                    }
                }
            MigrationProposal migrationProposal = {.task = closetDeadlineTask, .from = itsCPU, .to = endingCPU};
            _performMigration(migrationProposal);
        }
        else
        { // core has already some ready tasks
            ECOUT("\tcore busy, schedule ready task" << endl);
            _queues->schedule(endingCPU);
        }

        ECOUT(endl
              << endingCPU->pcu() << endl);
        ECOUT(".............................." << endl);
    }

    // virtual time ends
    void MRTKernel_Linux5_3_11_GRUB_PA::onReleasingIdle(CBServer *cbs)
    {
        ECOUT("EMRTK::" << __func__ << "()" << endl);
        CBServerCallingEMRTKernel *cbs1 = dynamic_cast<CBServerCallingEMRTKernel *>(cbs);
        CPU_BL *endingCPU = dynamic_cast<CPU_BL *>(cbs1->getJobLastCPU()); // forget U_active
        vector<AbsRTTask *> readyTasks = getReadyTasks(endingCPU);
        _numberVtimeEvents++;
        
        endingCPU->forgetUtilization_active(cbs);

        if (!readyTasks.empty())
        {
            ECOUT("\tCore already has some ready tasks. Scheduling one of those" << endl);
            _queues->schedule(endingCPU);
        }

        endingCPU->getIsland()->adjustFrequency(); // scale down to min freq by updating island _maxUtil
    }

    /// Callback, when a task gets running on a core
    void MRTKernel_Linux5_3_11_GRUB_PA::onTaskGetsRunning(AbsRTTask *t, CPU_BL *cpu)
    {
        cpu->setWorkload("bzip2");
    }

    /// Callback, when a CBS server ends a task
    void MRTKernel_Linux5_3_11_GRUB_PA::onTaskInServerEnd(AbsRTTask * t, CPU_BL * cpu, CBServer * cbs)
    {
        onEnd(cbs);
    }
    
    void MRTKernel_Linux5_3_11_GRUB_PA::onExecutingReleasing(CBServer * cbs)
    {
    }

    // discarded methods

    void MRTKernel_Linux5_3_11_GRUB_PA::onReplenishment(CBServerCallingEMRTKernel * cbs)
    {
    }

    // can frequency increase/decrease?
    EnergyMRTKernel::MigrationProposal MRTKernel_Linux5_3_11_GRUB_PA::migrateFromOtherIsland(CPU_BL * endingCPU)
    {
	    MigrationProposal migrationProposal = {.task = NULL, .from = NULL, .to = NULL};
	    return migrationProposal;
    }

    EnergyMRTKernel::MigrationProposal MRTKernel_Linux5_3_11_GRUB_PA::migrateFromOtherIsland_bff(CPU_BL * endingCPU, IslandType fromIslandType) const
    {
	    MigrationProposal migrationProposal = {.task = NULL, .from = NULL, .to = NULL};
	    return migrationProposal;
    }

    EnergyMRTKernel::MigrationProposal MRTKernel_Linux5_3_11_GRUB_PA::balanceLoad(CPU_BL * endingCPU, vector<AbsRTTask *> toBeSkipped)
    {
	    MigrationProposal migrationProposal = {.task = NULL, .from = NULL, .to = NULL};
	    return migrationProposal;
    }

    vector<AbsRTTask *> MRTKernel_Linux5_3_11_GRUB_PA::sortForBalancing(CPU_BL * from, CPU_BL * to)
    {
	    vector<AbsRTTask*> temp;
	    return temp;
    }

    bool MRTKernel_Linux5_3_11_GRUB_PA::isMigrationSafe(const MigrationProposal mp, bool frequencyCanChange /*= true*/) const
    {
	    return false;
    }

    bool MRTKernel_Linux5_3_11_GRUB_PA::isMigrationEnergConvenient(const MigrationProposal mp) const
    {
	    return false;
    }

    CBServerCallingEMRTKernel *MRTKernel_Linux5_3_11_GRUB_PA::addTaskAndEnvelope(AbsRTTask * t, const string &param)
    {
	    return NULL;
    }
}
