#ifndef __CBSERVER_H__
#define __CBSERVER_H__

#include <config.hpp>

#include <server.hpp>
#include <capacitytimer.hpp>
#include <list>
#include <sstream>
#include <exeinstr.hpp>
#include <rttask.hpp>

namespace RTSim { 
    using namespace MetaSim;

    class CBServer : public Server {
    public:
  typedef enum {ORIGINAL, REUSE_DLINE } policy_t;

  CBServer(Tick q, Tick p, Tick d, bool HR, const std::string &name, 
     const std::string &sched = "FIFOSched");

        virtual void newRun();
        void endRun();

        virtual Tick getBudget() const { return Q; }
        virtual Tick getPeriod() const { return P;}

        virtual Tick changeBudget(const Tick &n);

        Tick changeQ(const Tick &n);
        virtual double getVirtualTime();
        Tick get_remaining_budget() const; 

        policy_t get_policy() const { return idle_policy; }
        void set_policy(policy_t p) { idle_policy = p; }

// ------------------------------------ some useful functions

      /**
       Add a new task to this server, with parameters
       specified in params.
               
       @params task the task to be added
       @params the scheduling parameters

       @see Scheduler
      */
      virtual void addTask(AbsRTTask &task, const std::string &params = "") {
        Server::addTask(task, params);

        _yielding = false;
      }

        /// Returns all tasks currently in the scheduler
        vector<AbsRTTask*> getAllTasks() const { return sched_->getTasks(); }

        /// Returns all tasks currently in the server; synonym of getAllTasks()
        virtual vector<AbsRTTask*> getTasks() const {
          return getAllTasks();
        }

        /// Tells if scheduler currently holds any task. Function not that much tested!
        bool isEmpty() const {
          vector<AbsRTTask*> tasks = getTasks();
          unsigned int numTasks = tasks.size();
          return numTasks == 0;
        }

        bool isKilled() const { return _killed; }
        
        /// Tells if task is in scheduler
        bool isInServer(AbsRTTask* t) {
            if (dynamic_cast<Server*>(t))
              return false;

            bool res = sched_->isFound(t);
            return res;
        }

        bool isYielding() const { return _yielding; }

        /// The server yield the core where it's running
        void yield() {
          _yielding = true;
        }

        /// Returns the nominal utilization of the task inside (which changes over time)
        virtual double getUtilization() const
        {
          return double(Q) / double(getPeriod());
        }

        Tick getIdleEvent() const {
          return _idleEvt.getTime();
        }

        /// Synonim of getIdleEvent(), for easy of use
        Tick getEndOfVirtualTimeEvent() { return getIdleEvent(); }

        /// Server to human-readable string
        virtual string toString() const { 
          stringstream s; 
          s << "\ttasks: [ " << sched_->toString() << "]" << (isYielding() ? " yielding":"") <<
            " (Q:" << Q << ", P:" << getPeriod() << ")\tstatus: " << getStatusString();
          return s.str();
        }

    protected:
                
        /// from idle to active contending (new work to do)
        virtual void idle_ready();

        /// from active non contending to active contending (more work)
        virtual void releasing_ready();
                
        /// from active contending to executing (dispatching)
        virtual void ready_executing();

        /// from executing to active contenting (preemption)
        virtual void executing_ready();

        /// from executing to active non contending (no more work)
        virtual void executing_releasing();

        /// from active non contending to idle (no lag)
        virtual void releasing_idle();

        /// from executing to recharging (budget exhausted)
        virtual void executing_recharging();

        /// from recharging to active contending (budget recharged)
        virtual void recharging_ready();

        /// from recharging to active contending (budget recharged)
        virtual void recharging_idle();

        virtual void onReplenishment(Event *e);

        virtual void onIdle(Event *e);

        void prepare_replenishment(const Tick &t);
        
        void check_repl();

        /// True if CBS server has decided to yield core (= to leave it to ready tasks)
        bool _yielding;

      
    private:
        Tick recharging_time;
        int HR;
        
        /// replenishment: it is a pair of <t,b>, meaning that
        /// at time t the budget should be replenished by b.
        typedef std::pair<Tick, Tick> repl_t;

        /// queue of replenishments
        /// all times are in the future!
        std::list<repl_t> repl_queue;

        /// at the replenishment time, the replenishment is moved
        /// from the repl_queue to the capacity_queue, so 
        /// all times are in the past.
        std::list<repl_t> capacity_queue;

    protected:
        /// The total budget, in cycles
        Tick Q;

        Tick P;
        
        /// Server capacity, i.e. the remaining budget, in cycles
        Tick cap;
        
        /// Last time (tick) server has been executed - or has changed budget
        Tick last_time;

        /// A new event replenishment, different from the general
        /// "recharging" used in the Server class
        GEvent<CBServer> _replEvt;

        /// when the server becomes idle
        GEvent<CBServer> _idleEvt;

        CapacityTimer vtime;

        /** if the server is in IDLE, and idle_policy==true, the
            original CBS policy is used (that computes a new deadline
            as t + P) 
            If the server is IDLE and t < d and idle_policy==false, then 
            reuses the old deadline, and computes a new "safe" budget as 
            floor((d - vtime) * Q / P). 
        */
        policy_t idle_policy;

        /// Is CBS server killed?
        bool _killed = false;
    };

    class PSTrace;

    /**
      CBS server augmented specifically for EnergyMRTKernel.
      EMRTK. needs to know WCET to compute utilizations, and some callbacks to
      make decisions.

      Acronym: CBServer CEMRTK.
      */
    class CBServerCallingEMRTKernel : public CBServer {
    protected:

      virtual void idle_ready();

      /// same as the parent releasing_idle() but also calls EMRTKernel
      virtual void releasing_idle();

      /// from executing to active contenting (preemption)
      virtual void executing_ready();

      // same as parent, but also calls EMRTKernel
      virtual void executing_releasing();

      /// from executing to recharging (budget exhausted)
      virtual void executing_recharging();

      virtual void ready_executing();

    private:
      /// CPU where the server instance was before the last time, in the current server period
      CPU_BL* _jobLastCpu;

      /// The speed that has determined the current server capacity. Initially speed of BIG max freq (reference speed)
      double _speed = CPU_BL::REFERENCE_SPEED;

      /// The current utilization of the task inside (changes over time!)
      double _utilization_curr = -1.0;

      /// job already executed cycles
      Tick _alreadyExecutedCycles;

      /// Scaling factor used to decrease task WCET in favour of its wrapping CBS server. Default: WCET = budget
      double _scaling_factor = 1.0;

      /// true if the current instance of the only task contained in this server has been skipped
      bool _isCurInstanceSkipped = false;

      /// The plotsched instance of this task
      PSTrace *_pstrace = NULL;

      /// Returns the sole EnergyMRTKernel instance, to call its methods
      EnergyMRTKernel* energyMRTKernel() const;

    public:

      virtual double getSpeed() const {return _speed;}

      bool isAbort() const;

      CBServerCallingEMRTKernel(Tick q, Tick p, Tick d, bool HR, const std::string &name, 
        const std::string &sched = "FIFOSched") : CBServer(q,p,d,HR,name,sched) {};

      virtual Tick getBudget() const { return Tick::ceil(double(Q) / _speed); }

      /// CPU is only for debug 
      void changeBudget(double newSpeed, CPU* c);

      /// This method is not appropriate anymore since it doesn't take core speeds into account
      virtual Tick changeBudget(const Tick &n) { assert ("Do not call this method" == ""); return 0; }

      /// Kills the server and its task. It can stay killed since now on or only until task next period
      void killInstance(bool onlyOnce = true);

      /// Get task executed cycled. If alsoNow = true, SIMUL.getTime() is taken as descheduling time
      Tick getAlreadyExecutedCycles(bool alsoNow = false) const;

      AbsRTTask* getFirstTask() const {
        AbsRTTask* t = sched_->getFirst();
        return t;
      }

      Tick getEndEventTime() const;

      Tick getEndBandwidthEvent() const {
        return _bandExEvt.getTime();
      }

      Tick getReplenishmentEvent() const { return _replEvt.getTime(); }

      CPU* getJobLastCPU() const { return _jobLastCpu; }

      CPU* getProcessor() const;

      virtual CPU* getProcessor(const AbsRTTask* t) const { return getProcessor(); };

      /// Get remaining WCET in cycles of the enveloped task, i.e. frequency independent
      virtual Tick getRemainingWCETCycles() const;
      
      virtual vector<AbsRTTask*> getTasks() const {
        vector<AbsRTTask*> res;
        AbsRTTask* ft = getFirstTask();

        if (ft != NULL)
          res.push_back(ft);
        
        return res;
      }

      /// Returns the nominal utilization of the task inside (which changes over time)
      virtual double getUtilization() const;

      /// Get server WCET
      virtual double getWCET(double capacity) const {
        double res = ceil(double(Q) / capacity);
        assert (res > 0);
        return res;
      }

      string getTaskWorkload() const;

      bool isCurInstanceSkipped() const { return _isCurInstanceSkipped; }

      void updateCurrentUtilization();
      
      /// Arrival event of task of server
      virtual void onArrival(AbsRTTask *t);

      /// Task of server ends, callback
      virtual void onEnd(AbsRTTask *t);

      /// On deschedule event (of server - and of tasks in it)
      virtual void onDesched(Event *e);

      /// Server gets once more its full budget
      virtual void onReplenishment(Event *e);

      /// Callback. The kernel tells the server/task it has been migrated
      void onMigrated(CPU_BL* finalCPU);

      /// budget event. impossible to happen since no overrun is possible
      virtual void onBudgetExhausted(Event *e);

      /// Set scaling factor x in WCET = Q * x, where Q is generated by taskgen.py
      void setScalingFactor(double scal) { _scaling_factor = scal; }

      /*
        Skip current task instance (see you at your next arrival!)
        The kernel instance is needed to automatically remove the task from its scheduler, if you don't pass NULL
      */
      void skipInstance(EnergyMRTKernel* kernel);

      void setPSTrace(PSTrace* p) { _pstrace = p; }

      /// Object to human-readable string
      virtual string toString() const {
        string s = "CBSCEMRTK " + getName() + ". " + CBServer::toString() + ". util_cur: " + to_string(getUtilization()) + ". _speed: " + to_string(_speed);  
        return s;
      }

      /// Prints (cout) all events of CBS Server
      virtual void printEvts() const {
        ECOUT( endl << toString() );
        ECOUT( "_bandExEvt: " << _bandExEvt.getTime() << ", " );
        ECOUT( "_rechargingEvt: " << _rechargingEvt.getTime() << ", " );
        ECOUT( "_idleEvt: " << _idleEvt.getTime() << ", " );
        ECOUT( endl << endl );
      } 

    };

}


#endif
