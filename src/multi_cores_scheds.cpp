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

#include <multi_cores_scheds.hpp>
#include <mrtkernel.hpp>
#include <energyMRTKernel.hpp>


namespace RTSim {
    using namespace MetaSim;
    using namespace std;

    MultiCoresScheds::MultiCoresScheds(MRTKernel* k, vector<CPU*>& cpus, vector<Scheduler*>& scheds, const string& name)
            : Entity(name) {
        assert(cpus.size() == scheds.size());
        
        _kernel = k;
        _beginEvts = new BeginDispatchMultiEvt*[cpus.size()];
        _endEvts   = new EndDispatchMultiEvt*  [cpus.size()];

        for (int i = 0; i < cpus.size(); i++) {
            scheds.at(i)->setKernel(k);
            _queues[cpus.at(i)] = scheds.at(i);
            _beginEvts[i] = NULL;
            _endEvts[i]   = NULL;
        }
        
    }

    void MultiCoresScheds::addTask(AbsRTTask* t, CPU* c, const string &params) {
        _queues[c]->addTask(t, params);
        if (dynamic_cast<RRScheduler*>(_queues[c]))
          dynamic_cast<RRScheduler*>(_queues[c])->notify(t);
    }

    void MultiCoresScheds::insertTask(AbsRTTask* t, CPU* c) {
        try {
            _queues[c]->insert(t);
        } catch(RTSchedExc &e) {
            // core schedulers/queues don't know tasks until this point
            ECOUT( "Receiving this error once per task is ok" << endl );
            addTask(t,c,"");
            MultiCoresScheds::insertTask(t,c);
        }
    }

    void MultiCoresScheds::removeFirstFromQueue(CPU* c) {
        AbsRTTask *t = getFirst(c);
        removeFromQueue(c, t);
    }
    
    void MultiCoresScheds::removeFromQueue(CPU* c, AbsRTTask* t) {
        assert(c != NULL); assert(t != NULL);
        if (_queues[c]->isFound(t))
            _queues[c]->extract(t);
        dropEvt(c, t);
    }

    AbsRTTask* MultiCoresScheds::getFirst(CPU* c) {
        assert(c != NULL);

        AbsRTTask *tt = _queues[c]->getFirst();
        return tt;

        // AbsRTTask *t = NULL;
        // vector<AbsRTTask*> tasks = getAllTasksInQueue(c);
        // for (AbsRTTask* tt : tasks) {
        //     if (t == NULL)
        //         t = tt;
        //     else
        //         if (tt->getPeriod() < t->getPeriod())
        //             t = tt;
        // }

        // return t;
    }

    AbsRTTask* MultiCoresScheds::getFirstReady(CPU* c) {
        assert(c != NULL);
        // assumes there is only 1 CBS server per core

        AbsRTTask *t = _queues[c]->getTaskN(1);
        return t;
    }

    vector<AbsRTTask*> MultiCoresScheds::getAllTasksInQueue(CPU* c) const {
        vector<AbsRTTask*> tasks;

        Scheduler* s = _queues.at(c);
        AbsRTTask *t;
        int i = 0;
        while( (t = s->getTaskN(i)) != NULL) {
            if (dynamic_cast<CBServer*>(t)->isYielding()) {
                i++;
                continue;
            }

            tasks.push_back(t);
            i++;
        }

        return tasks;
    }

    void MultiCoresScheds::empty(CPU* c) {
        for (CBServer* s : _kernel->getServers())
            removeFromQueue(c, s);

        while (!isEmpty(c)) {
            removeFirstFromQueue(c);
        }
    }

    bool MultiCoresScheds::isEmpty(CPU* c) {
        return countTasks(c) == 0;
    }

    CPU* MultiCoresScheds::isInAnyQueue(const AbsRTTask* t) {
        for (const auto& q : _queues) {
            vector<AbsRTTask*> tasks = getAllTasksInQueue(q.first);
            for (AbsRTTask* tt : tasks)
                if (tt == t)
                    return q.first;
        }
        return NULL;
    }

    unsigned int MultiCoresScheds::countTasks(CPU* c) {
        int i = 0;
        while( _queues[c]->getTaskN(i) != NULL )
            i++;
        return i;
    }

    // ---------------------------------------- scheduling & context switch

    void MultiCoresScheds::schedule(CPU* c) {
        assert(c != NULL);

        AbsRTTask *highestPriorityTask = getFirst(c);

        ECOUT( "MCS::" << __func__ << "() " << (highestPriorityTask == NULL ? "" : "highestPriorityTask=" + highestPriorityTask->toString() + " on ") << c->getName() << ", t=" << SIMUL.getTime() << endl );
        
        // deschedule running task or the one I planned to schedule first
        if (getRunningTask(c) != NULL)
          if (getRunningTask(c) != highestPriorityTask)
            makeReady(c);

        if (_beginEvts[c->getIndex()] != NULL) {
          ECOUT( "\tCancelling events (begin ctx switch)" << endl );
          dropEvt(c, _beginEvts[c->getIndex()]->getTask());
        }
        if (_endEvts[c->getIndex()] != NULL) {
          ECOUT( "\tCancelling events (end ctx switch)" << endl );
          dropEvt(c, _endEvts[c->getIndex()]->getTask());
        }

        // schedule highest priority task
        if (highestPriorityTask != NULL && highestPriorityTask != getRunningTask(c))
          makeRunning(highestPriorityTask, c); // set task (begin of) ctx switch evt
    }

    void MultiCoresScheds::makeRunning(AbsRTTask* t, CPU* c) {
        assert(c != NULL); assert(t != NULL);
        
        Tick when = SIMUL.getTime();
        if (isContextSwitching(c))
          when = _endEvts[c->getIndex()]->getTime();
        dropEvt(c, t);
        postBeginEvt(c, t, when);
        ECOUT( "t = " << SIMUL.getTime() << ", ctx switch set at " << double(when) << " for " << taskname(t) << endl );
    }

    /// Transition from running to ready on core c
    void MultiCoresScheds::makeReady(CPU* c) {
        /* - Drop possible end (ctx switch) event. Occurs if oldTask has
         * ctx switch overhead and another, more important task is
         * scheduled during it.
         * - Ready tasks don't have ctx events.
         */
        AbsRTTask *oldTask = getRunningTask(c);
        dropEvt(c, oldTask);

        _running_tasks.erase(c);
        oldTask->deschedule();
    }

    void MultiCoresScheds::postEvt(CPU* c, AbsRTTask* t, Tick when, bool endevt) {
        assert(c != NULL); assert(t != NULL); assert(when >= SIMUL.getTime());

            CPU_BL* cc = dynamic_cast<CPU_BL*>(c);
        if (endevt) {
            EndDispatchMultiEvt *ee  = new EndDispatchMultiEvt(*_kernel, *c);
            ee->post(when);
            ee->setTask(t);
            ee->setDisposable(false);
            _endEvts[cc->getIndex()] = ee;
        }
        else {
            BeginDispatchMultiEvt *ee = new BeginDispatchMultiEvt(*_kernel, *c);
            ee->post(when);
            ee->setTask(t);
            ee->setDisposable(false);
            _beginEvts[cc->getIndex()] = ee;
        }

    }

    void MultiCoresScheds::dropEvt(CPU* c, AbsRTTask* t) {
        assert(c != NULL); assert(t != NULL);

        if (_beginEvts[c->getIndex()] != NULL && _beginEvts[c->getIndex()]->getTask() == t) {
            _beginEvts[c->getIndex()]->drop();
            delete _beginEvts[c->getIndex()];
            _beginEvts[c->getIndex()] = NULL;
        }

        if (_endEvts[c->getIndex()] != NULL && _endEvts[c->getIndex()]->getTask() == t) {
            _endEvts[c->getIndex()]->drop();
            _endEvts[c->getIndex()] = NULL;
        }
    }

    bool MultiCoresScheds::isContextSwitching(CPU* c) const {
      bool ret = _endEvts[c->getIndex()] != NULL;
      return ret;
    }

    void MultiCoresScheds::onBeginDispatchMultiFinished(CPU* c, AbsRTTask* newTask, Tick overhead) {
      assert(c != NULL); assert(newTask != NULL); assert(double(overhead) >= 0.0);

      dropEvt(c, newTask);
      postEndEvt(c, newTask, SIMUL.getTime() + overhead);
    }

    void MultiCoresScheds::onEndDispatchMultiFinished(CPU* c, AbsRTTask* t) {
      assert(c != NULL); assert(t != NULL);

      _running_tasks[c] = t;
      dropEvt(c, t);
    }

    void MultiCoresScheds::onEnd(AbsRTTask *t, CPU* c) {
      assert(c != NULL); assert(t != NULL);

      ECOUT( "\t" << c->getName() << " has now wl: " << c->getWorkload() << ", speed: " << c->getSpeed() << endl << endl );

      AbsRTTask *running = getRunningTask(c);

      removeFromQueue(c, t);
      if (running != NULL) // I may kill a ready task..
        makeReady(c);
    }

    void MultiCoresScheds::onMigrationFinished(AbsRTTask* t, CPU* original, CPU* final) {
        assert(t != NULL); assert(original != NULL); assert(final != NULL);
        ECOUT( "\t\tt=" << SIMUL.getTime() << ", MCS::" << __func__ << "() migration performed of " << t->toString() << " from " << original->toString() << " to " << final->toString() << endl );

        try {
            removeFromQueue(original, t);
            insertTask(t, final);
            // schedule(final); this func is needed

            _migrationNumber++;
        } catch(RTSchedExc &e) {
            insertTask(t, final);
            onMigrationFinished(t, original, final);
        }
    }

    void MultiCoresScheds::onExecutingReleasing(CPU* cpu, CBServer *cbs) {
        ECOUT( "t=" << SIMUL.getTime() << " MCS::" << __func__ << "() for " << cbs->getName() << endl );
        // assert(cbs != NULL); assert(cpu != NULL);

        // ECOUT( "\tCBS server has now #tasks=" << cbs->getTasks().size() << (cbs->getTasks().size() > 0 ? " - first one: " + cbs->getTasks().at(0)->toString() : "") << endl );
    }

    /// cbs recharging itself
    CPU* MultiCoresScheds::onReplenishment(CBServer *cbs) {
        ECOUT( "t=" << SIMUL.getTime() << " MCS::" << __func__ << "() for " << cbs->getName() << endl );

        CPU* formerCPU = dynamic_cast<CBServerCallingEMRTKernel*>(cbs)->getJobLastCPU();

        CPU *cpu = getProcessorRunning(cbs);
        if (cpu != NULL) // server might be with no task => already ready
            makeReady(cpu);
        return formerCPU;
    }
  
  /**
   * Function called only when RRScheduler is used. It informs
   * the queues manager that a task has finished its round => remove from
   * queue and take next task
   */
    void MultiCoresScheds::onRound(AbsRTTask *finishingTask, CPU *c) {
        if (c == NULL) // task has just finished its WCET
            return;
        schedule(c);
    }

    /// When task in CBS server ends
    void MultiCoresScheds::onTaskInServerEnd(AbsRTTask* t, CPU* cpu, CBServer* cbs) {
        ECOUT( "MCS::" << __func__ << "() for " << cbs->getName() << endl );
        assert(t != NULL); assert(cbs != NULL); assert(cpu != NULL);
    }

    string MultiCoresScheds::toString() {
        return "MultiCoresScheds toString().";
    }

}
