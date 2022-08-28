/***************************************************************************
    begin                : Thu Apr 24 15:54:58 CEST 2003
    copyright            : (C) 2003 by Giuseppe Lipari
    email                : lipari@sssup.it
 ***************************************************************************/
 /***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RTLIB2_0_ENERGYMRTKERNEL_H
#define RTLIB2_0_ENERGYMRTKERNEL_H

#include <config.hpp>
#include "mrtkernel.hpp"
#include "rttask.hpp"
#include "multi_cores_scheds.hpp"
#include "cbserver.hpp"

#define _ENERGYMRTKERNEL_DBG_LEV    "EnergyMRTKernel"

#define ROUTINE_ON_NON_SCHEDULABLE(t, n)                                                                                                  \
            if (EMRTK_ABORT_ON_NOT_SCHEDULABLE) {                                                                                         \
              cerr    << "t="<< SIMUL.getTime() << ". Job does not find inside the min utilization core. Cores too utilized?" << endl;    \
              cerr    << "----------------------" << endl;                                                                                \
              cerr    <<  "This may be no bug. Case " << n << ", more info in log file." << endl;                                         \
              cerr    <<  "Task causing abort: " << t->toString() << endl;                                                                \
              printState(1,1);                                                                                                            \
              abort();                                                                                                                    \
            }                                                                                                                             \
            else {                                                                                                                        \
                cerr    << "----------------------" << endl;                                                                              \
                cerr    << "t=" << SIMUL.getTime() << ". Job does not find inside the min utilization core. Cores too utilized?" << endl; \
                cerr    << "This may be no bug. Case " << 1 << ", more info in log file." << endl;                                        \
                cerr    << t->toString() << " instance will be skipped, task'll come again in its next period." << endl;                  \
                printState(1,1);                                                                                                          \
                _tasksNotSchedulable++;                                                                                                   \
                                                                                                                                          \
                dynamic_cast<CBServerCallingEMRTKernel*>(t)->skipInstance(this);                                                          \
                                                                                                                                          \
                cerr    << "----------------------" << endl;                                                                              \
            }

namespace RTSim {

    /**
        \ingroup sched

        Extension of MultiCoresScheds for Big-Little.
        Tasks are also bound to the desired OPP.

        Notice this is not a scheduler, its a class to manage cores queues
        of ready and running tasks.
     */
    class EnergyMultiCoresScheds : public MultiCoresScheds {
    private:
        vector<pair<CPU_BL*, unsigned int>> _opps;

    public:
        EnergyMultiCoresScheds(MRTKernel* kernel, vector<CPU*> &cpus, vector<Scheduler*> &s, const string& name);

        ~EnergyMultiCoresScheds() { ECOUT( __func__ << endl ); }

        /// called when a new task is inserted in the taskset
        void onNewTasksetElement(AbsRTTask* t) {
           _opps[t->getTaskNumber()] = pair<CPU_BL*, unsigned int>(NULL, 0);
        }

        /**
           To be called when migration finishes. 
           It deletes ctx switch events on the original core and
           prepares for context switch on the final core (= chosen after migration).

           Migration is specifically meant for big-littles. It updates task WCET
        */
        virtual void onMigrationFinished(AbsRTTask* t, CPU* original, CPU* final);

        virtual EnergyMRTKernel* getKernel() const;

        /// Remove the first task of a core queue
        virtual void removeFirstReadyFromQueue(CPU_BL* c) {
            AbsRTTask *t = getFirstReady(c);
            removeFromQueue(c, t);
        }

        /// Remove a specific task from a core queue
        virtual void removeFromQueue(CPU_BL* c, AbsRTTask* t) {
            MultiCoresScheds::removeFirstFromQueue(c);
            _opps[t->getTaskNumber()] = pair<CPU_BL*, unsigned int>(NULL, 0);
        }

        /// Empties a core queue
        virtual void empty(CPU_BL* c) {
            vector<AbsRTTask*> tasks = getAllTasksInQueue(c);
            MultiCoresScheds::empty(c);
            for (AbsRTTask* t : tasks)
                _opps[t->getTaskNumber()] = pair<CPU_BL*, unsigned int>(NULL, 0);
        }

        virtual void endRun() {
            MultiCoresScheds::endRun();
        }

        virtual unsigned int getOPP(CPU_BL* c) {
            unsigned int maxOPP = 0;
            for (const auto& e : _opps)
              if ( e.first->getName() == c->getName() && e.second > maxOPP)
                maxOPP = e.second;
            return maxOPP;
        }

        /// Add a task to the queue of a core. Then, tells the c's island
        virtual void insertTask(AbsRTTask* t, CPU* c);

        string toString(CPU_BL* c, int* executingTasks) const;

        virtual string toString();

    };


    class TextTrace;
    /**
        \ingroup kernel

        An implementation of a real-time multi processor kernel with
        global scheduling and a policy to smartly select CPU_BLs on big-LITTLE
        architectures. It contains:

        - a pointer to a list of CPU_BLs managed by the kernel;

        - a pointer to a Scheduler, which implements the scheduling
          policy;

        - a pointer to a Resource Manager, which is responsible for
          resource access related operations and thus implements a
          resource allocation policy;

        - a map of pointers to CPU_BL and task, which keeps the
          information about current task assignment to CPU_BLs;

        - the set of tasks handled by this kernel.

        This implementation is not quite general: it lets the user of this
        class the freedom to adopt any scheduler derived form
        Scheduler and a resource manager derived from ResManager or no
        resource manager at all.  The kernel for a multiprocessor
        system with different CPU_BLs can be also be simulated. It is up
        to the instruction class to implement the correct duration of
        its execution by asking the kernel of its task the speed of
        the processor on which it's scheduled.
        It is not that general because the aim was to make everything work
        as expected. That's why there are so many dynamic_cast<CPU_BL*>,
        even when CPU is enough. 

        @see absCPU_BLFactory, Scheduler, ResManager, AbsRTTask
    */
    class EnergyMRTKernel : public MRTKernel {

    protected:
        struct ConsumptionTable {
            double cons;
            CPU_BL* cpu;
            int opp;
        };

        /// A task envoleped inside a server
        struct EnvelopedTask {
            AbsRTTask *_task;
            CBServerCallingEMRTKernel  *_server;
        };

        struct MigrationProposal {
            AbsRTTask *task;
            CPU_BL *from;
            CPU_BL *to;
        };

        // little, big (order matters for speed)
        Island_BL* _islands[2];

        /// Map of tasks and their own server, where they are enveloped
        map<AbsRTTask*, CBServerCallingEMRTKernel*> _envelopes; 

        /// cores queues, containing ready and running tasks for each core
        EnergyMultiCoresScheds *_queues = NULL;

        /// for debug, if you want to force a certain choice of cores and frequencies and how many times to dispatch
        map<AbsRTTask*, tuple<CPU_BL*, unsigned int, unsigned int>> _m_forcedDispatch;

        /// for debug, list of discarded tasks and how many times to discard them
        map<AbsRTTask*, tuple<unsigned int>> _discardedTasks;

        /// number of performed virtual time expiration events, for debug
        unsigned int _numberVtimeEvents = 0;

        /// number of performed end events, for debug
        unsigned int _numberEndEvents = 0;

        /// # non-schedulable tasks because system's over-utilized (placeTask() choses a CPU but EDF admission test fails)
        unsigned int _tasksNotSchedulable = 0;

        /// the texttrace instance for tracing tasks/cpus events
        TextTrace* _textTracer;

        // ----------------------------------- Migrations

        /// Balance load by migration. Tasks in toBeSkipped will be skipped
        MigrationProposal balanceLoad (CPU_BL *endingCPU, vector<AbsRTTask*> toBeSkipped = {});

        /// Generates migration proposal for P-EDF-FF
        MigrationProposal migrateFromRightCoreFF(CPU_BL* endingCPU);

        /// Proposes a migration from Big to Little. Tasks in toBeSkipped will be skipped
        MigrationProposal migrateFromOtherIsland (CPU_BL *endingCPU);

        /// Migrate biggest ready job from emptiest big
        MigrationProposal migrateFromBig_bfeb (CPU_BL *endingCPU) const;

        /// Migrate biggest ready job from fullest big
        MigrationProposal migrateFromOtherIsland_bff (CPU_BL *endingCPU, IslandType fromIslandType) const;

        /// After verifying the proposal, it performs it
        bool _performMigration(MigrationProposal);

        /// Returns the power consumption that a migration would implicate in the "from" or "to" core only
        double getPowerDelta(MigrationProposal mp, string where = "from") const;

        /// Is migration energetically convenient? True if energy consumption decreases or is equal between cores
        bool isMigrationEnergConvenient(const MigrationProposal mp) const;

        /// Does migration break schedulability on the ending core?
        bool isMigrationSafe(const MigrationProposal mp, bool frequencyCanChange = true) const;


        /**
        * CPU_BL choice from the table of consumptions (not sorted).
        * It tries to spread tasks on CPU_BLs if they have the same energy consumption
        */
        void chooseCPU_BL(AbsRTTask* t, vector<ConsumptionTable> iDeltaPows);

        /// Returns the CBS servers enveloping the periodic tasks
        vector<AbsRTTask*> getEnvelopers(vector<AbsRTTask*> ptasks) const {
          if (!EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED) return ptasks;

          vector<AbsRTTask*> envelopes;
          for (AbsRTTask *t : ptasks)
            if (isCBServer(t))
              envelopes.push_back(t);
            else
              envelopes.push_back(getEnveloper(t));
          return envelopes;
        }

        /// Get island big/little
        Island_BL* getIsland(IslandType island) const { return _islands[island]; }

        bool isCBServer(AbsRTTask* t) const { return dynamic_cast<CBServerCallingEMRTKernel*>(t) != NULL; }

        /**
           Tells if a task is to be descheduled on a CPU

           This method has been introduced for RRScheduler, which needs to tell wheather a
           task has finished its quantum
        */
        bool isToBeDescheduled(CPU_BL* p, AbsRTTask *t);

        /**
         * Implements the policy of leaving little 3 free, just in case a task with high WCET arrives,
         * risking to be forced to schedule it on big cores, increasing power consumption.
         */
        void leaveLittle3(AbsRTTask *t, std::vector<ConsumptionTable> iDeltaPows, CPU_BL*& chosenCPU_BL);

        /// Returns a possible migration to endingCPU. Tasks in toBeSkipped will be skipped. Implements migration mechanism on task end.
        MigrationProposal getTaskToMigrateInto(CPU_BL* endingCPU, vector<AbsRTTask*> toBeSkipped = {});

        /// Pulls a task into endingCPU_BL. It finally performs the migration. Tasks in toBeSkipped will be skipped
        bool migrateInto(CPU_BL* endingCPU_BL, vector<AbsRTTask*> toBeSkipped = {});

        /// Performs a temporary migration, i.e. one that last only until a task arrives on endingCPU
        bool migrateTemporarily(CPU_BL* endingCPU) {
          return false; // disabled
        }

        /// Sorts tasks for the two cores to have utilizations as close as possible
        vector<AbsRTTask*> sortForBalancing(CPU_BL* from, CPU_BL* to);

        /// When choosing where to dispatch task, try to keep it on its former core. -1 if impossible to keep former core
        int tryKeepingOldCPU (AbsRTTask* t, vector<struct ConsumptionTable> &iDeltaPows) const;

        /// is task admissible on that CPU using G-EDF?
        bool isAdmissible(AbsRTTask* t, CPU_BL* c) const;

        static bool EMRTK_DISABLE_DEBUG_TRYONCPU_BL                     ; /// Do you need to debug tryTaskOnCPU_BL(), which is verbous?
    public:
        static bool EMRTK_BALANCE_ENABLED                               ; /// Can't imagine disabling it, but so policy is in the list :)
        static bool EMRTK_LEAVE_LITTLE3_ENABLED                         ;
        static bool EMRTK_MIGRATE_ENABLED                               ; /// Migrations enabled? (if disabled, its dependencies won't work, e.g. EMRTK_CBS_MIGRATE_AFTER_END)
        static bool EMRTK_CBS_YIELD_ENABLED                             ;

        static bool EMRTK_CBS_MIGRATE_AFTER_END                         ; /// After a task ends its WCET, can you migrate? Needs EMRTK_MIGRATE_ENABLED
        static bool EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END        ; /// After task ends its virtual time, there can be migrations (requires EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED)
        static bool EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG         ; /// When migrating from Big island, pick the most empty big core and in there pick the biggest ready task fitting in little
        static bool EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG          ; /// When migrating from Big island, pick the fullest big core and in there pick the biggest ready task fitting in little
        static bool EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_LITTLE       ; /// Useful since not always littles consume less than bigs for all frequencies
        static bool EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG_ONLY_FIRST; // Consider only the very fullest big core with migr. policy EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG
        static bool EMRTK_TEMPORARILY_MIGRATE_VTIME                     ; /// Enables temporary (and fake) migrations on vtime end evt, i.e. migration that only last until until a task starts on endingCPU
        static bool EMRTK_TEMPORARILY_MIGRATE_END                       ; /// As VTIME, but on task end evt (killed, end WCET, etc.)

        static bool EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED               ; /// CBS server enveloping periodic tasks?

        static bool EMRTK_TRY_KEEP_OLD_CPU                              ; /// When a job arrives, try to keep its former core to avoid migration costs
        static bool EMRTK_ENLARGE_WCET                                  ; /// Should WCET slightly enlarged to cover rounding problems?
        static bool EMRTK_ENLARGE_WCET_10_PERC                          ; /// CBS budget = wrapped task WCET + 10%
        static bool EMRTK_ENLARGE_WCET_UNIF                             ; /// CBS budget = wrapped task WCET * [rand unif. distrib.]. task WCET is reduced

        static bool EMRTK_ABORT_ON_NOT_SCHEDULABLE                      ;

        static bool EMRTK_FORGET_UACTIVE_ON_CORE_FREE                   ; /// Necessary to make experimets still work
        static bool EMRTK_FREQUENCY_SCALE_DOWN                          ; /// Is frequency scaling down of island possible at all?

        static bool EMRTK_PLACEMENT_LITTLE_FIRST                        ; /// if the scheduler can choose between little and big, it'll choose little island

        static bool EMRTK_FIT_BIG_LITTLE_CHOSEN                         ; /// Needed to measure placem. alg. performance

        static bool EMRTK_PRINT_PLACEMENT_TIME                          ; /// If true, placement delay is written into file
        static bool EMRTK_PLACEMENT_P_EDF_WF;                           ; /// If true, places the tasks according to the P-EDF-WorstFit logic (default)
        static bool EMRTK_PLACEMENT_P_EDF_BF;                           ; /// If true, places the tasks according to the P-EDF-BestFit logic
        static bool EMRTK_PLACEMENT_P_EDF_FF;                           ; /// If true, places the tasks according to the P-EDF-FirstFit logic


        EnergyMRTKernel(Scheduler *s, const string &name = "") : MRTKernel(s, "") {};

        /**
          * Kernel with scheduler s and CPU_BLs CPU_BLs.
          * qs are the schedulers you want for MultiCoresScheds, the queues of cores.
          *
          * @see MultiCoresScheds
          */
        EnergyMRTKernel(vector<Scheduler*> &qs, Scheduler *s, Island_BL* big, Island_BL* little, const string &name = "");

        virtual ~EnergyMRTKernel() {
          cout << __func__ << endl;
          delete _islands[0];
          delete _islands[1];

          if (_queues != NULL)
            delete _queues;
          _queues = NULL;
        }

        virtual string getClassName() const { return "EnergyMRTKernel"; }

        /**
          Adds a periodic task into scheduler and returns the CBS server enveloping it.
          Call this function instead of addTask() for periodic tasks.
          It also supports CBS servers, which don't get enveloped.
          */
        CBServerCallingEMRTKernel* addTaskAndEnvelope(AbsRTTask *t, const string &param = "");

        unsigned int getNumberEndEvents() const { return _numberEndEvents; }
        unsigned int getNumberVtimeEvents() const { return _numberVtimeEvents; }

        int getTasksNonSchedulable() const { return _tasksNotSchedulable; }

        unsigned int getFrequencyIncreasesLittle() const { return _islands[0]->getFrequencyIncreases(); }
        unsigned int getFrequencyIncreasesBig() const { return _islands[1]->getFrequencyIncreases(); }
        Island_BL* getIslandLittle() const { return _islands[0]; }
        Island_BL* getIslandBig() const { return _islands[1]; }
        void       setIslandLittle(Island_BL* island) { _islands[0] = island; }
        void       setIslandBig(Island_BL* island) { _islands[1] = island; }
      	Scheduler* getScheduler() { return _sched; }

        /**
         * Returns the residual nominal capacity of a core 
         * (i.e. referred to core max speed, and not its current OPP, if isNom=true)
         */
        double getResCapNomCore(CPU_BL* c, bool isNom = true) const;

        /**
           This is different from the version we have in MRTKernel: here you decide a
           processor for arrived tasks. See also @chooseCPU_BL() and @dispatch(CPU_BL*)

           This function is called by the onArrival and by the
            activate function. When we call this, we first select a
            processor, chosen smartly, for arrived tasks, then we call the other dispatch,
            confirming the dispatch process.
         */
        virtual void dispatch();

        /**
           This function only calls dispatch(CPU_BL*) and assigns a task to a CPU_BL,
           which should be actually done by dispatch(CPU_BL*) itself or onEndDispatchMulti(),
           but the last one is only called after dispatch(), and I need the assignment
           CPU_BL - task to be done before for getTask(CPU_BL*) to work
         */
        void dispatch(AbsRTTask *t, CPU *p);

        /**
            Dispatching a task on a given CPU_BL.

            This is different from the version we have on RTKernel,
            since we may need to specify on which CPU_BL we have to
            select a new task (for example, in the onEnd() and
            suspend() functions). In the onArrival() function,
            instead, we still do not know which processor is
            free!

            This is not used at all here because MRTkernel decides here
            to be task context switch in the current simulation or after a while.
            But that's because dispatch() just selects a free CPU and puts the
            new task there. EnergyMRTKernel, instead, allows CPUs to have a queue
            of tasks. Info about task is needed
         */
        virtual void dispatch(CPU* c) {}

        virtual bool isGEDF() const { return false; }

        /**
            Returns the abs time (=cycles) already executed by the task.
            If alsoNow = true, SIMUL.getTime() is taken as descheduling time.
          */
        virtual double getAlreadyExecutedCycles(AbsRTTask* task, bool alsoNow = false) const {
          assert (task != NULL);

          double res = dynamic_cast<CBServerCallingEMRTKernel*>(task)->getAlreadyExecutedCycles();
          return res;
        }

        /// Returns the enveloped RTask of CBS server
        AbsRTTask* getEnveloped(AbsRTTask* cbs) const {
          assert (dynamic_cast<CBServer*>(cbs)); assert (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED);

          for (const auto& elem : _envelopes)
            if (elem.second == cbs)
              return elem.first;
          return NULL;
        }

        /// Returns the CBS server enveloping the periodic task t
        CBServerCallingEMRTKernel* getEnveloper(AbsRTTask* t) const {
          if (!EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED || isCBServer(t)) return dynamic_cast<CBServerCallingEMRTKernel*>(t);

          for (auto &elem : _envelopes)
            if (elem.first == t)
              return elem.second;

          return NULL;
        }

        /// Tells on what core queue a task is ready (and not running)
        virtual CPU_BL* getProcessorReady(AbsRTTask* t) const;

        /// Get core where task is running
        virtual CPU_BL *getProcessorRunning(AbsRTTask *t) const {
          if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED && dynamic_cast<PeriodicTask*>(t))
            t = getEnveloper(t);

          CPU *c = _queues->getProcessorRunning(t);
          CPU_BL *cc = dynamic_cast<CPU_BL*>(c);
          return cc;
        }

        /// Get core where task is dispatched (either running and ready)
        virtual CPU_BL* getProcessorReadyRunning(AbsRTTask* t) const 
        {
          if (EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED && dynamic_cast<PeriodicTask*>(t))
            t = getEnveloper(t);

          CBServerCallingEMRTKernel* cbs = dynamic_cast<CBServerCallingEMRTKernel*>(t);
          if (cbs != NULL && cbs->isKilled())
            return dynamic_cast<CPU_BL*>(cbs->getProcessor(cbs));

          CPU *c = _queues->getProcessor(t);
          CPU_BL* cc = dynamic_cast<CPU_BL*>(c);
          return cc;
        }

        /// Get core where task is running
        virtual CPU_BL *getProcessor(AbsRTTask *t) const {
          throw string("method disabled");
          return NULL;
        }

        /// Get all processors, in all islands
        vector<CPU_BL*> getProcessors() const { 
            vector<CPU_BL*> CPU_BLs;
            for (CPU_BL* c : getIslandLittle()->getProcessors())
                CPU_BLs.push_back(c);
            for (CPU_BL* c : getIslandBig()->getProcessors())
                CPU_BLs.push_back(c);
            return CPU_BLs;
        }

        /// Get processors of an island
        vector<CPU_BL*> getProcessors(IslandType island) const {
            return getIsland(island)->getProcessors();
        }

        /// For debug. Returns the layer managing CPUs queues/schedulers
        EnergyMultiCoresScheds* getEnergyMultiCoresScheds() const {
          return _queues;
        }

        double getUtilization(AbsRTTask* t) const;

        /// Returns the set of tasks in the runqueue of CPU_BL c, but the runnning one, ordered by DL (300, 400, ...)
        virtual vector<AbsRTTask*> getReadyTasks(CPU_BL* c) const;

        /**
         *  Returns a pointer to the task which is executing on given
         *  CPU_BL (NULL if given CPU_BL is idle)
         */
        virtual AbsRTTask* getRunningTask(CPU* c) const;

        virtual vector<AbsRTTask*> getAllTasks(CPU* c) const {
          return _queues->getAllTasksInQueue(c);
        }

        /// Is task temporarily migrated on core to
        bool isTaskTemporarilyMigrated(AbsRTTask* t, CPU_BL* to) const {
          return false; // disabled
        }

        /**
         * Begins the dispatch process (context switch). The task is dispatched, but not
         * executed yet. Its execution on its CPU_BL starts with onEndDispatchMulti()
         */
        virtual void onBeginDispatchMulti(BeginDispatchMultiEvt* e);

        /**
         *  First a task is dispatched, but not executed yet, in the
         *  onBeginDispatchMulti. Then, in the onEndDispatchMulti, its execution starts
         *  on the processor.
         */
        virtual void onEndDispatchMulti(EndDispatchMultiEvt* e);

        /// called when OPP changes on an island
        void onOppChanged(unsigned int oldOPP, unsigned int newOPP, Island_BL* island);

        ///Invoked when a task ends
        virtual void onEnd(AbsRTTask* t);

        /// Callback called when a task on a CBS CEMRTK. goes executing -> releasing
        virtual void onExecutingReleasing(CBServer* cbs);

        /// Callback, when CBS server is killed. It expects the tasks inside the CBS server still alive.
        void onCBSKilled(AbsRTTask* t, CPU_BL* cpu, CBServerCallingEMRTKernel* cbs) {
          assert (cpu != NULL); assert(cbs != NULL);
          ECOUT( "EMRTK::" << __func__ << "() for " << cbs->toString() << endl );

          _queues->onTaskInServerEnd(t, cpu, cbs); // save util active
          _queues->onEnd(cbs, cpu);
        }

        /// Callback called when a task on a CBS CEMRTK goes releasing -> idle (virtual time ends)
        /// I expect that cbs has finished its WCET or it finishes right now
        virtual void onReleasingIdle(CBServer *cbs);

        /// cbs recharging itself
        void onReplenishment(CBServerCallingEMRTKernel* cbs);

        /// Callback, when a CBS server ends a task
        virtual void onTaskInServerEnd(AbsRTTask* t, CPU_BL* cpu, CBServer* cbs);

        /// Callback, when a task gets running on a core
        virtual void onTaskGetsRunning(AbsRTTask *t, CPU_BL* cpu);

        /**
         * Specifically called when RRScheduler is used, it informs the kernel that
         * finishingTask has just finished its round (slice)
         */
        void onRound(AbsRTTask* finishingTask);

        virtual void placeTask(AbsRTTask* t, vector<CPU_BL*> toBeExcluded = {});

        void placeTask_WF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded = {});

        void placeTask_BF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded = {});

        void placeTask_FF(AbsRTTask* t, vector<CPU_BL*> toBeExcluded = {});

        /// update tasks nominal utilization according to the remaining period
        void updateIslandTasksUtilization(Island_BL* island);

        void setTextTracer(TextTrace* tracer) { this->_textTracer = tracer; }

        virtual void newRun() {
            MRTKernel::newRun();
            if (_queues != NULL)
              _queues->newRun();
        }

        virtual void endRun() {
            MRTKernel::endRun();
            if (_queues != NULL)
              _queues->endRun();
        }

        /// Kernel to string
        virtual string toString() const {
          return "EnergyMRTKernel";
        }

 /// ---------------------------------------------------- to debug internal functions...

        void test();

        virtual string time();

        void printEnvelopes() const {
          for (auto& elem : _envelopes)
            ECOUT( elem.first->toString() << " -> " << elem.second->toString() << endl );
        }

        void printBool(bool b);

        void printPolicies() const {
          ECOUT( "EMRTK_BALANCE_ENABLED: " << EMRTK_BALANCE_ENABLED << endl );
          ECOUT( "EMRTK_LEAVE_LITTLE3_ENABLED: " << EMRTK_LEAVE_LITTLE3_ENABLED << endl );
          ECOUT( "EMRTK_MIGRATE_ENABLED: " << EMRTK_MIGRATE_ENABLED << endl ); /// Migrations enabled? (if disabled, its dependencies won't work, e.g. EMRTK_CBS_MIGRATE_AFTER_END)
          ECOUT( "EMRTK_CBS_YIELD_ENABLED: " << EMRTK_CBS_YIELD_ENABLED << endl );

          ECOUT( "EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED: " << EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED          << endl ); /// CBS server enveloping periodic tasks?
          ECOUT( "EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END: " << EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END   << endl ); /// After task ends its virtual time, it can be migrated (requires CBS_ENVELOPING)
          ECOUT( "EMRTK_CBS_MIGRATE_AFTER_END: " << EMRTK_CBS_MIGRATE_AFTER_END << endl );
        }

        virtual void printState() { printState(false); }

        virtual void printState(bool alsoQueues, bool alsoCBSStatus = false, bool alsoRunningTasks = false);

        bool manageForcedDispatch(AbsRTTask*);

        void addForcedDispatch(AbsRTTask *t, CPU_BL *c, unsigned int opp, unsigned int times = 1);

        bool manageDiscartedTask(AbsRTTask *t) {
          assert (t != NULL);

          if (_discardedTasks.find(t) != _discardedTasks.end()) {
            get<0>(_discardedTasks[t])--;
            if (get<0>(_discardedTasks[t]) == 0)
              _discardedTasks.erase(t);
            return true;
          }

          return false;
        }

        void printAllTasks(string filename) const;

    };

    /**
     * SCHED_DEADLINE version with GRUB_PA.
     * Support to big.little.
     * 
     * A task arrives (its period begins).
     * Dispatch it onto a free core if possible.
     * Or if not available, dispatch it into
     * the core with latest deadline.
     * After that, take the core with
     * highest utilization in the island
     * and scale down the frequency.
     * If the the utilization of ready and 
     * running task on a core overcomes
     * the core maximum speed, then select
     * the highest OPP available.
     * Each core has a ready/running queue.
     * 
     * When a task ends, save its active utilization.
     * 
     * When the virtual time ends, scale down the
     * core active utilization and adjust island frequency.
     */
    class MRTKernel_Linux5_3_11_GRUB_PA : public EnergyMRTKernel
    {
    public:
        MRTKernel_Linux5_3_11_GRUB_PA(vector<Scheduler*> &qs, Scheduler *s, Island_BL* big, Island_BL* little, const string &name = "");

        ~MRTKernel_Linux5_3_11_GRUB_PA() {
          cout << __func__ << endl;
        }

        // this is G-EDF but with core queues too
        virtual bool isGEDF() const { return true; }

        virtual string getClassName() const { return "MRTKernel_Linux5_3_11_GRUB_PA"; }

      protected:

        virtual void dispatch();

        virtual void placeTask(AbsRTTask *t, vector<CPU_BL *> toBeExcluded = {});

        virtual void onEnd(AbsRTTask *cbs);

        virtual void onExecutingReleasing(CBServer* cbs);

        virtual void onReleasingIdle(CBServer* cbs);

        virtual void onTaskInServerEnd(AbsRTTask* t, CPU_BL* cpu, CBServer* cbs);

        // --------------------- deleted methods

        virtual void onReplenishment(CBServerCallingEMRTKernel *cbs) ;

        /// Callback, when a task gets running on a core
        virtual void onTaskGetsRunning(AbsRTTask *t, CPU_BL* cpu) ;

        // can frequency increase/decrease?
        virtual EnergyMRTKernel::MigrationProposal migrateFromOtherIsland (CPU_BL *endingCPU) ;

        virtual EnergyMRTKernel::MigrationProposal migrateFromOtherIsland_bff(CPU_BL* endingCPU, IslandType fromIslandType) const ;

        virtual EnergyMRTKernel::MigrationProposal balanceLoad(CPU_BL *endingCPU, vector<AbsRTTask*> toBeSkipped) ;

        virtual vector<AbsRTTask*> sortForBalancing(CPU_BL* from, CPU_BL* to) ;

        virtual bool isMigrationSafe(const MigrationProposal mp, bool frequencyCanChange /*= true*/) const;

        virtual bool isMigrationEnergConvenient(const MigrationProposal mp) const;

        virtual CBServerCallingEMRTKernel* addTaskAndEnvelope(AbsRTTask *t, const string &param) ;
    };
}

#endif //RTLIB2_0_ENERGYMRTKERNEL_H
