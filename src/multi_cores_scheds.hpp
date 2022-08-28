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

#ifndef SIMPLE_EXAMPLE_MULTI_SCHED_HPP
#define SIMPLE_EXAMPLE_MULTI_SCHED_HPP

#include <config.hpp>

#include <string>
#include "entity.hpp"
#include "mrtkernel.hpp"
#include <exeinstr.hpp>

#include <rrsched.hpp>
#include <edfsched.hpp>

namespace RTSim {

    using namespace MetaSim;
    using namespace std;

    struct cpuComparator {
      bool operator()(CPU *lhs, CPU *rhs) const
      {
        return lhs->getIndex() < rhs->getIndex();
      }
    };

    /**
        \ingroup sched

        An implementation of multi-scheduler, i.e. every core has a queue, implemented as scheduler.
        Notice it is not a scheduler, but an interface to
        manage cores queues, which are implemented as schedulers. This class is also named
        Multi Cores Queues.

        The typical scenario is:
        you have N cores and you need a queue per core. Each queue can be
        modelled as a scheduler. Schedulers are supposed to be equal!

        This interface has been introduced for EnergyMRTkernel, managing
        big-littles, where for energetic reasons the scheduler (the kernel in RTSim)
        might decide to dispatch many tasks to a certain core.

        Notice that this layer (laid above a multicore kernel) does not de/schedule() tasks,
        it only post begin and end context switches.
        To enforce consistency between kernel and multi-scheduler, kernel needs to provide,
        for each relevant function, task and core where it know that core is currently scheduled.

        Before using this layer, please disable (disable()) the kernel scheduler. It will still
        generate onArrival() => dispatch() events, but other callbacks will get disabled.

        This implementation is general and you can specialize it.

        @see CPU, AbsRTTask, EnergyMultiCoresScheds
     */
    class MultiCoresScheds : public MetaSim::Entity {
    protected:
        struct MigrationTable {
            string evtType;         // scheduled, descheduled, cf (change freq)
            bool isMigrated;        // is this a migration event (task ready on original core -> running on final core)
            bool isDiscarted;       // signals this is before task has ended, thus it shall be discarted in computation of already exec'd time
            bool isFrequencyChanged;// is this migration entry here because of a frequency change?

            Tick time;              // scheduling event time
            string workload;        // workload type
            double frequency_init;  // frequency when task is scheduled (not used in calculus, next tasks may change it)
            CPU* cpu;               // CPU where task is migrated into

            double frequency_end;   // frequency when task is descheduled
            double speed;           // speed of cpu (for debug)
        };

        /// cores queues, ordered according to a scheduling policy. On each queue, tasks can be running, ready or under context switch 
        map<CPU*, Scheduler*, cpuComparator> _queues;

        /// running task of a queue, for all queues. Running tasks are also in _queues
        map<CPU*, AbsRTTask*>  _running_tasks;

        /// cores have a queue of ready=dispatching tasks. These are the context switch events (don't confuse with _endEvt of instr.)
        BeginDispatchMultiEvt** _beginEvts;
        EndDispatchMultiEvt**   _endEvts;

        /// MRTKernel this the queue are related to
        MRTKernel* _kernel;

        // CBS: task -> cpu, virtual time (= time to forget its active utilization), u_active nominal
        map<AbsRTTask*, tuple<CPU*, Tick, double>> _active_utilizations;


        /// Post begin event for a task on a core
        void postBeginEvt(CPU* c, AbsRTTask* t, Tick when) {
            postEvt(c, t, when, false);
        }
        /// Post end event for a task on a core
        void postEndEvt(CPU* c, AbsRTTask* t, Tick when) {
            postEvt(c, t, when, true);
        }

        /// Drops all context switch events of task t on core c
        void dropEvt(CPU* c, AbsRTTask* t);

        /// Transition from ready to running for task t on core c
        virtual void makeRunning(AbsRTTask* t, CPU* c);

        /// Transition from running to ready on core c
        virtual void makeReady(CPU* c);

        /// Add a task to the scheduler of a core. @see insertTask
        virtual void addTask(AbsRTTask* t, CPU* c, const string& params);

    private:

        /// Posts a context switch event
        void postEvt(CPU* c, AbsRTTask* t, Tick when, bool endevt);

        /// Number of performed migrations
        unsigned int _migrationNumber = 0;

    public:

        MultiCoresScheds() : Entity("trash multisched"){};

        MultiCoresScheds(MRTKernel *k, vector<CPU*> &cpus, vector<Scheduler*> &s, const string& name);

        ~MultiCoresScheds() {
            for (auto c : _queues) {
                vector<AbsRTTask*> tt = getAllTasksInQueue(c.first);
                for (AbsRTTask* t : tt)
                  delete t;
                delete c.second;
            }

            delete[] _beginEvts;
            delete[] _endEvts;
        }


        /// Counts the tasks of a core queue
        unsigned int countTasks(CPU* c);

        /// Empties a core queue
        virtual void empty(CPU* c);

        /// True if the core queue is empty (i.e., no ready and running tasks on c)
        bool isEmpty(CPU* c);

        // ------------------------------------------------- migration history
        unsigned int getMigrationNumber() const { return _migrationNumber; }

        // ----------------------------------------------- other methods
        
        /// Get scheduler of a core
        Scheduler* getScheduler(CPU* c) {
            return _queues[c];
        }

        /// Get the first (running or ready) task of a core queue
        virtual AbsRTTask* getFirst(CPU* c);

        /// Get the first ready task of a core or NULL if no ready tasks are available
        virtual AbsRTTask* getFirstReady(CPU* c);

        /// Get processor where task is running
        CPU* getProcessorRunning(const AbsRTTask *t) const {
          for (const auto& elem : _running_tasks)
            if (elem.second == t)
              return elem.first;
          return NULL;
        }

        /// Get processor where task is ready
        CPU* getProcessorReady(const AbsRTTask *t) const {
          for (auto& elem : _queues) {
              vector<AbsRTTask*> tasks = getAllTasksInQueue(elem.first);
              for (AbsRTTask* task : tasks)
                if (task == t) {
                  CPU* cpu = elem.first;
                  return cpu;
                }
          }
          return NULL;
        }

        /// get core where task is dispatched (either running and ready)
        CPU* getProcessor(const AbsRTTask *t) const {
          CPU *cpu = getProcessorRunning(t);
          if (cpu == NULL)
            cpu = getProcessorReady(t);  
          return cpu;
        }

        /// Get running task for core c. Tasks under context switch are not considered as running
        virtual AbsRTTask* getRunningTask(CPU* c) const {
            assert(c != NULL);
            AbsRTTask* running = NULL;
            
            if (_running_tasks.find(c) != _running_tasks.end())
              running = _running_tasks.at(c);
            
            return running;
        }

        /// Get the current HP task on the queue of core c. It can be ready or running
        AbsRTTask *getHighestPriority(CPU *c) const
        {
          assert(c != NULL);
          AbsRTTask *hp = NULL;
          
          vector<AbsRTTask*> all = getAllTasksInQueue(c);
          if (all.size() > 0)
              hp = all.at(0);

          return hp;
        }

        /// Get all tasks of a core queue
        vector<AbsRTTask*> getAllTasksInQueue(CPU* c) const;

        vector<AbsRTTask*> getReadyTasks(CPU* c) const {
            assert(c != NULL);

            vector<AbsRTTask*> tasks = getAllTasksInQueue(c);
            AbsRTTask* running = getRunningTask(c);
            AbsRTTask* ctxswitch = getTaskUnderContextSwitch(c);

            // remove the running task and the task under context switch
            if (running != NULL || ctxswitch != NULL)
              for (int i = 0; i < tasks.size(); i++)
                  if (tasks.at(i) == running || tasks.at(i) == ctxswitch) {
                      tasks.erase(tasks.begin() + i);
                      break;
                  }

            return tasks;
        }

        /// Returns tasks under context switch of a given cpu
        AbsRTTask* getTaskUnderContextSwitch(CPU* c) const {
          assert (c != NULL);
          AbsRTTask* task = NULL;

          if (_beginEvts[c->getIndex()] != NULL) {
            task = _beginEvts[c->getIndex()]->getTask();
            assert (_endEvts[c->getIndex()] == NULL);
          }
          else if (_endEvts[c->getIndex()] != NULL) {
            task = _endEvts[c->getIndex()]->getTask();
            assert (_beginEvts[c->getIndex()] == NULL);
          }

          return task;
        }

        /**
         * Add a task to the queue of a core.
         */
        virtual void insertTask(AbsRTTask* t, CPU* c);

        /// Its CPU if task in dispatched in any queue, else NULL
        virtual CPU* isInAnyQueue(const AbsRTTask* t);

        /// True if CPU is under a context switch (= there is a task in the limbo between beginDispatchMulti - endDispatchMulti)
        bool isContextSwitching(CPU* c) const;

        void onBeginDispatchMultiFinished(CPU* c, AbsRTTask* newTask, Tick overhead);

        void onEndDispatchMultiFinished(CPU* c, AbsRTTask* t);

        /// Kernel signals end of task event (WCET finished)
        void onEnd(AbsRTTask *t, CPU* c);

        /**
           To be called when migration finishes. 
           It deletes ctx switch events on the original core and
           prepares for context switch on the final core (= chosen after migration).

           Migration is not specifically meant for big-littles, it can also be
           a task movement between 2 cores. That's why this function is here
        */
        virtual void onMigrationFinished(AbsRTTask* t, CPU* original, CPU* final);

        void onExecutingReleasing(CPU* cpu, CBServer *cbs);

        /// cbs recharging itself
        CPU* onReplenishment(CBServer *cbs);
      
      /**
       * Function called only when RRScheduler is used. It informs
       * the queues manager that a task has finished its round => remove from
       * queue and take next task
       */
        void onRound(AbsRTTask *finishingTask, CPU *c);

        /// When task in CBS server ends
        void onTaskInServerEnd(AbsRTTask* t, CPU* cpu, CBServer* cbs);

        /// Remove the first task of a core queue. Also removes its ctx evt
        virtual void removeFirstFromQueue(CPU* c);

        /// Remove a specific task from a core queue. Also removes its ctx evt
        virtual void removeFromQueue(CPU* c, AbsRTTask* t);

        /// schedule first task of core queue, i.e. posts its context switch event/time
        void schedule(CPU* c);

        /// Executes next ready task on core c
        void yield(CPU* c) {
            assert(c != NULL); // running task might have already ended 

            ECOUT( "\tCore status: " << _queues[c]->toString() << endl );
            AbsRTTask *nextReady = getFirstReady(c);
            if (nextReady != NULL) {
              ECOUT( "\tYielding in favour of " << nextReady->toString() << endl );
              AbsRTTask* runningTask = getRunningTask(c);
              if (runningTask != NULL) {
                makeReady(c);
                runningTask->deschedule();
              }
              makeRunning(nextReady, c);
            }
        }

        virtual void newRun() {}

        virtual void endRun() {
            for (auto& e : _queues)
                empty(e.first);
        }

        virtual string toString();

    };

} // namespace RTSim
#endif //SIMPLE_EXAMPLE_MULTI_SCHED_HPP
