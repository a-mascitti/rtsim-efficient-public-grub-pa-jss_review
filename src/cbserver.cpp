#include "cbserver.hpp"
#include "energyMRTKernel.hpp"
#include "cpu.hpp"
#include <energyMRTKernel.hpp>
#include <ps_trace.hpp>

#include <cassert>

#define dcout cout << "t=" << SIMUL.getTime() << ", " << typeid(this).name() << "::" << __func__ << "() line " << __LINE__ << ", " << toString() << ", cap " << cap

namespace RTSim {

	using namespace MetaSim;

	CBServer::CBServer(Tick q, Tick p,Tick d, bool HR, const std::string &name,
			   const std::string &s) :
		Server(name, s),
		Q(q),
		P(p),
		cap(0),
		last_time(0),
		HR(HR),
		_replEvt(this, &CBServer::onReplenishment, 
		 Event::_DEFAULT_PRIORITY - 1),
		_idleEvt(this, &CBServer::onIdle, Event::_DEFAULT_PRIORITY - 1), // standard version of RTSim uses _DEFAULT_PRIORITY
		vtime(),
		idle_policy(ORIGINAL)
	{
        abs_dline = 0;
		DBGENTER(_SERVER_DBG_LEV);
		DBGPRINT(s);
	}

	void CBServer::newRun()
	{
		ECOUT( "CBServer::" << __func__ << "()" << endl );
		DBGENTER(_SERVER_DBG_LEV);
		DBGPRINT_2("HR ", HR);
		cap = Q;
		last_time = 0;
		recharging_time = 0;
		status = IDLE;
		capacity_queue.clear();
		if (vtime.get_status() == CapacityTimer::RUNNING) vtime.stop();
		vtime.set_value(0);
	}

	double CBServer::getVirtualTime()
	{
		DBGENTER(_SERVER_DBG_LEV);
		DBGPRINT("Status = " << status_string[status]);
		double vt;
		if (status == IDLE)
		   vt = double(SIMUL.getTime());
		else 
		   vt = vtime.get_value();
		return vt;
	}

	void CBServer::endRun()
	{
	}

	void CBServer::idle_ready()
	{
		// ECOUT( "CBS::" << __func__ << "() Q=" << Q << " " << endl );
		DBGENTER(_SERVER_DBG_LEV);
		assert (status == IDLE);
		status = READY;

		cap = 0;

		if (idle_policy == REUSE_DLINE && SIMUL.getTime() < getDeadline()) {
			double diff = double(getDeadline() - SIMUL.getTime()) * 
			double(Q) / double(P);
			cap = Tick(std::floor(diff));
		}

		if (cap == 0) {
			cap = Q;
			//added relative deadline
			abs_dline = SIMUL.getTime() + P;
			DBGPRINT_2("new deadline ", abs_dline);
			//setAbsDead(d);
		}
		vtime.set_value(SIMUL.getTime());
		DBGPRINT_2("Going to active contending ",SIMUL.getTime());
	}
	
	/*I should compare the actual bandwidth with the assignedserver Q
	 *  and postpone deadline and full recharge or just use what is
	 *  left*/
	// this should not be necessary. 
	// In fact, the releasing state should be the state in which: 
	// 1) the server is not executing
	// 2) the if (condition) is false.
	// in other words, it should be possible to avoid the if.
	void CBServer::releasing_ready()
	{
		ECOUTD( " cap " << cap << endl );
		DBGENTER(_SERVER_DBG_LEV);
		status = READY;
		_idleEvt.drop();
		DBGPRINT("FROM NON CONTENDING TO CONTENDING");
	}

	void CBServer::ready_executing()
	{
		ECOUTD( endl );
		DBGENTER(_SERVER_DBG_LEV);

		status = EXECUTING;
		last_time = SIMUL.getTime();
		vtime.start((double)P/double(Q));
		ECOUT( "\tCBS::" << __func__ << "() vtime=" << getVirtualTime() << " setting vtime with " << P << "/" << Q << "=" << (double(P) / double(Q)) << endl );
		DBGPRINT_2("Last time is: ", last_time);
		ECOUT( "\tCBS::" << __func__ << "(). _bandExEvt posted at " << last_time << "+" << cap << "=" << last_time + cap << " for " << toString() << endl ); 
		_bandExEvt.post(last_time + cap);
	}


	/*The server is preempted. */
	void CBServer::executing_ready()
	{
		ECOUTD( endl );
		DBGENTER(_SERVER_DBG_LEV);
		//assert(isEmpty());

		status = READY;
		cap = cap - (SIMUL.getTime() - last_time);
		vtime.stop();
		_bandExEvt.drop();

		ECOUTD( " cap = " << cap << endl );
	}

	/*The sporadic task ends execution*/
	void CBServer::executing_releasing()
	{
		ECOUTD( endl );
		DBGENTER(_SERVER_DBG_LEV);
	
		if (status == EXECUTING) {
			cap = cap - (SIMUL.getTime() - last_time);
			vtime.stop();
			_bandExEvt.drop();
		}
		if (vtime.get_value() <= double(SIMUL.getTime())) 
			status = IDLE;
		else {
			_idleEvt.post(Tick::ceil(vtime.get_value()));
			status = RELEASING;
		}        
		DBGPRINT("Status is now XXXYYY " << status_string[status]);
	}

	void CBServer::releasing_idle()
	{
		DBGENTER(_SERVER_DBG_LEV);
		status = IDLE;
	}

	/*The server has no more bandwidth The server queue may be empty or not*/
	void CBServer::executing_recharging()
	{
		ECOUTD( endl );
		DBGENTER(_SERVER_DBG_LEV);

		_bandExEvt.drop();
		vtime.stop();

		DBGPRINT_2("Capacity before: ", cap);
		DBGPRINT_2("Time is: ", SIMUL.getTime());
		DBGPRINT_2("Last time is: ", last_time);
		DBGPRINT_2("HR: ", HR);
		if (!HR) { // Hard Reservation, Abeni 1998
			cap = Q;
			abs_dline += P;
			//setAbsDead(d);
			DBGPRINT_2("Capacity is now: ", cap);
			DBGPRINT_2("Capacity queue: ", capacity_queue.size());
			DBGPRINT_2("new_deadline: ", abs_dline);
			status=READY;
			_replEvt.post(SIMUL.getTime());
			ECOUTD( "CBS::" << __func__ << "(). _replEvt posted at " << SIMUL.getTime() << endl );
			}
		else
		{ // CBSCEMRTK:
			cap = 0;
			_replEvt.post(abs_dline);
			ECOUTD( "CBS::" << __func__ << "(). _replEvt posted at " << abs_dline << endl );
			abs_dline += P;
			//setAbsDead(d);
			status=RECHARGING;
		}

		//inserted by rodrigo seems we do not stop the capacity_timer
			// moved up
		// vtime.stop();
		
		DBGPRINT("The status is now " << status_string[status]);
	}

	/*The server has recovered its bandwidth and there is at least one task left in the queue*/
	void CBServer::recharging_ready()
	{
		ECOUTD( " cap=" << cap << endl );
		DBGENTER(_SERVER_DBG_LEV);
		status = READY;
	}

	void CBServer::recharging_idle()
	{
		ECOUTD( " cap=" << cap << endl );
		assert(false);
	}

	void CBServer::onIdle(Event *e)
	{
		ECOUTD( "CBS::" << __func__ << "()" << ", cap " << cap << endl );
		DBGENTER(_SERVER_DBG_LEV);
		releasing_idle();
	}

	void CBServer::onReplenishment(Event *e)
	{
		ECOUTD( " cap=" << cap << endl );
		DBGENTER(_SERVER_DBG_LEV);
		
		_replEvt.drop();

		DBGPRINT_2("Status before: ", status);
		DBGPRINT_2("Capacity before: ", cap);

		if (status == RECHARGING || 
			status == RELEASING || 
			status == IDLE) {
			cap = Q;//repl_queue.front().second;
			if (sched_->getFirst() != NULL) {
				recharging_ready();
				kernel->onArrival(this);
			}
			else if (status != IDLE) {
				if (double(SIMUL.getTime()) < vtime.get_value()) {
					status = RELEASING;
					_idleEvt.post(Tick::ceil(vtime.get_value()));
				}
				else status = IDLE;
				
				currExe_ = NULL;
				sched_->notify(NULL);
			}
		}
		else if (status == READY || status == EXECUTING) {
			if (sched_->getFirst() == this) {
			}

			//       repl_queue.pop_front();
			//capacity_queue.push_back(r);
			//if (repl_queue.size() > 1) check_repl();
			//me falta reinsertar el servidor con la prioridad adecuada

			ECOUTD( " cap = " << cap << endl );
	}

		DBGPRINT_2("Status is now: ", status_string[status]);
		DBGPRINT_2("Capacity is now: ", cap);
		ECOUTD( " cap=" << cap << endl );
	}

	Tick CBServer::changeBudget(const Tick &n)
	{
		assert (false);
		ECOUTD( " n (new budget)=" << n << ", Q (old budget)=" << Q << endl );
		Tick ret = 0;
		DBGENTER(_SERVER_DBG_LEV);

		if (n > Q) {
			DBGPRINT_4("Capacity Increment: n=", n, " Q=", Q);
			cap += n - Q;
			if (status == EXECUTING) {
				DBGPRINT_3("Server ", getName(), " is executing");
				cap = cap - (SIMUL.getTime() - last_time);
				_bandExEvt.drop();
				vtime.stop();
				last_time = SIMUL.getTime();
				_bandExEvt.post(last_time + cap);
				vtime.start((double)P/double(n));
				DBGPRINT_2("Reposting bandExEvt at ", last_time + cap);
			}
			Q = n;
			ret = SIMUL.getTime();
		}
		else if (n == Q) {
			DBGPRINT_2("No Capacity change: n=Q=", n);
			ret = SIMUL.getTime();
		}
		else if (n > 0) {
			DBGPRINT_4("Capacity Decrement: n=", n, " Q=", Q);
			if (status == EXECUTING) {
				DBGPRINT_3("Server ", getName(), " is executing");
				cap = cap - (SIMUL.getTime() - last_time);
				last_time = SIMUL.getTime();
				DBGVAR(cap);
				_bandExEvt.drop();
				vtime.stop();
			}
			Q = n;

			if (status == EXECUTING) {
				vtime.start(double(P)/double(Q));
				ECOUTD(  "CBS::" << __func__ << "() - EXEC. setting vitime with " << P << "/" << Q << endl );
				DBGPRINT("Server was executing");
				if (cap == 0) {
					DBGPRINT("capacity is zero, go to recharging");
					_bandExEvt.drop();
					_bandExEvt.post(SIMUL.getTime());
				}
				else {
					DBGPRINT_2("Reposting bandExEvt at ", last_time + cap);    
					_bandExEvt.post(last_time + cap);
				}
			}
		}
		return ret;
	}

	// needed for tests
	Tick CBServer::get_remaining_budget() const
    	{
		double dist = (double(getDeadline()) - vtime.get_value()) * 
	   	double(Q) / double(P) + 0.00000000001;
	
		return Tick::floor(dist);
    	}

	void CBServerCallingEMRTKernel::changeBudget(double newSpeed, CPU* c) {
		ECOUT( "t=" << SIMUL.getTime() << " begin of " << __func__ << "() " << toString() << " endevt=" << getEndEventTime() << " newSpeed=" << newSpeed << endl );
		assert (newSpeed > 0.0 && newSpeed <= CPU_BL::REFERENCE_SPEED);
		DBGENTER(_SERVER_DBG_LEV);

		if (SIMUL.getTime() == getEndEventTime())
			return;

        if (status == EXECUTING) {
          executing_ready();
          _speed = newSpeed;
          if (cap >= 0) {
            ready_executing();
          }
          else {
            executing_recharging();
          }
        }
        else if (status == READY)
        	_speed = newSpeed;

		ECOUTD( "t=" << SIMUL.getTime() << ", end of " << __func__ << "() " << toString() << ", status " << getStatusString() << " final cap " << cap << " initial speed " << _speed << " new speed " << newSpeed << endl );
	}

	EnergyMRTKernel* CBServerCallingEMRTKernel::energyMRTKernel() const {
		return dynamic_cast<EnergyMRTKernel*>(kernel); 
	}

	void CBServerCallingEMRTKernel::killInstance(bool onlyOnce) {
		ECOUTD( endl );

		Task* t = dynamic_cast<Task*>(getFirstTask());
		_jobLastCpu = dynamic_cast<CPU_BL*>(t->getCPU());
		assert(t != NULL); assert (_jobLastCpu != NULL);

		_killed = true;

		executing_releasing();
		status = EXECUTING;
		yield();
		_bandExEvt.drop();
		_replEvt.drop();
		_rechargingEvt.drop();

		t->killInstance();
		ECOUTD(  endl << endl << "Kill event is now " << t->killEvt.toString() << endl << endl );
		t->killEvt.doit();
		ECOUTD(  "Kill event is dropped? " << t->killEvt.toString() << endl );

		status = RELEASING;
		_dispatchEvt.drop();
	}

	Tick CBServerCallingEMRTKernel::getEndEventTime() const {
		Task *task = dynamic_cast<Task*>(getFirstTask());
        assert (task != NULL);
        Tick ee = dynamic_cast<ExecInstr*>(task->getInstrQueue().at(0).get())->_endEvt.getTime();
		return ee;
	}

	string CBServerCallingEMRTKernel::getTaskWorkload() const {
        return "bzip2";
	}

	CPU* CBServerCallingEMRTKernel::getProcessor() const {
		return _jobLastCpu;
	}

	double CBServerCallingEMRTKernel::getUtilization() const {
        return _utilization_curr;
	}

	Tick CBServerCallingEMRTKernel::getAlreadyExecutedCycles(bool alsoNow) const {
		Tick res = _alreadyExecutedCycles;
		return res;
	}

	Tick CBServerCallingEMRTKernel::getRemainingWCETCycles() const {
		Tick total   = Tick::ceil(getWCET(CPU_BL::REFERENCE_SPEED));
		Tick already = getAlreadyExecutedCycles();
		Tick rem = total - already;
		assert (rem >= 0);
		return rem;
	}

	void CBServerCallingEMRTKernel::updateCurrentUtilization() {
		ECOUT( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "() for " << toString() << " from " << getUtilization() );
		Tick remaining_period = getDeadline() - SIMUL.getTime();
        _utilization_curr  = double(getRemainingWCETCycles()) / double(remaining_period);
        assert( _utilization_curr >= -0.00000001 ); assert (remaining_period > 0);
        ECOUT( " to " << getUtilization() << endl );
	}

	void CBServerCallingEMRTKernel::skipInstance(EnergyMRTKernel* kernel) {
		ECOUT( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "() for " << toString() << endl );
		PeriodicTask* task = dynamic_cast<PeriodicTask*>(getFirstTask());
		
		// drop task events
		task->skipInstance();
		
		sched_->extract(task);
		currExe_ = nullptr;
		_dispatchEvt.drop();

		_bandExEvt.drop();
		_replEvt.drop();
		_rechargingEvt.drop();
		status = IDLE;
		_isCurInstanceSkipped = true;

		if (kernel != NULL) {
			kernel->getScheduler()->extract(this);
			assert(!kernel->getScheduler()->isInQueue(this));
		}

		cerr << toString() << endl;
		// expected behaviour: job disappears from now until its next period and reappears from its next period
	}

	bool CBServerCallingEMRTKernel::isAbort() const
	{
		Task* t = dynamic_cast<Task *>(getFirstTask());
		
		if (t == NULL)
			return false;

		return t->isAbort();
	}

	// job in server arrives when period begins
	void CBServerCallingEMRTKernel::onArrival(AbsRTTask *t) {
        ECOUT("t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "(). " << t->toString() << (status == IDLE ? " IDLE" : " RELEASING") << " soon READY." << endl );
        
		if ( !isAbort() && (status == RELEASING || status == EXECUTING || status == READY) )
		{
			ECOUT ("Task is still executing or ready with soft deadlines. See you at the next period!" << endl);
			return;
		}
		
		_killed = false;
        _yielding = false;
		_isCurInstanceSkipped = false;
        _utilization_curr = ceil(getWCET(CPU_BL::REFERENCE_SPEED)) / double(getPeriod());
        _alreadyExecutedCycles = Tick(0);
        assert (status == IDLE || status == RELEASING); // RELEASING when period=wcet (here for exps)
        Server::onArrival(t); // dispatch to CPU
        assert (status == READY || isCurInstanceSkipped());
		_jobLastCpu = energyMRTKernel()->getProcessorReadyRunning(this);

		if (_pstrace != NULL && getProcessor() != NULL)
			_pstrace->write(t, to_string(getProcessor()->getIndex()) + "\tCREATION\tI");
    }

	void CBServerCallingEMRTKernel::onDesched(Event *e) {
		if (!isEmpty() && getEndEventTime() == SIMUL.getTime()) {
			ECOUT( "CBSCEMRTK::" << __func__ << "() for " << toString() << ". Skipping because endEvent should be more important. EndEvt=" << getEndEventTime() << endl );
			return;
		}

		ECOUT( "CBSCEMRTK::" << __func__ << "() for " << toString() << endl );
		if (isEmpty())
		    yield();
		else // could happend with non-periodic tasks
		    Server::onDesched(e);
	}

	// the only job within server ends. Server only contains one job 
	void CBServerCallingEMRTKernel::onEnd(AbsRTTask *t) {
		ECOUTD( "" << endl << "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "() for " << toString() << " on " << getProcessor()->getName() << endl );
		CPU_BL* cpu = dynamic_cast<CPU_BL*>(getProcessor(t));
		Server::onEnd(t); // extract ending task from the CBS queue

		_jobLastCpu = energyMRTKernel()->getProcessorReadyRunning(this);
		
		if (getEndBandwidthEvent() == SIMUL.getTime())
			ECOUT ( "CBSCEMRTK::" << __func__ << "() coincides with end of bandwidth event. Warning" << endl );

		// notify EMRTK the task endEvt.
		// In turn, it decreases core util,
		// saves acive util
		// and schedules eventual ready task on same core
		EnergyMRTKernel* emrtk = energyMRTKernel();
		if (emrtk != NULL) emrtk->onTaskInServerEnd(t, cpu, this);
		ECOUT( "" << endl );
	}

	void CBServerCallingEMRTKernel::onMigrated(CPU_BL* finalCPU) {
		assert (status == READY); assert (!isEmpty());
		_jobLastCpu = finalCPU;
		updateCurrentUtilization();
    }

	void CBServerCallingEMRTKernel::onReplenishment(Event *e) {
		ECOUT( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "()" << endl );
		CBServer::onReplenishment(e);

		EnergyMRTKernel* emrtk = energyMRTKernel();
		if (emrtk != NULL) emrtk->onReplenishment(this);
	}

	void CBServerCallingEMRTKernel::onBudgetExhausted(Event *e) {
		ECOUTD( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "() " << toString() << endl );
		abort();
	}

	void CBServerCallingEMRTKernel::executing_recharging() {
		ECOUTD( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "()" << endl );
		CBServer::executing_recharging();
	}

	// task ends and waits for virtual time to expire
	void CBServerCallingEMRTKernel::executing_releasing() {
		ECOUT( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "()" << endl );
		DBGENTER(_SERVER_DBG_LEV);
	
		if (status == EXECUTING) {
			cap = cap - Tick::ceil(double(SIMUL.getTime() - last_time) * _speed);
			vtime.stop();
			_bandExEvt.drop();
		}

		if (Tick::ceil(vtime.get_value()) <= SIMUL.getTime()) {
			status = IDLE;
			ECOUT( __func__ << "() t=" << SIMUL.getTime() << ", vtime=" << Tick::ceil(vtime.get_value()) << " => in the past => not posted for" << toString() << endl );
		}
		else {
			_idleEvt.post(Tick::ceil(vtime.get_value()));
			ECOUT( __func__ << "() t=" << SIMUL.getTime() << ", posted vtime=_idleEvt=" << Tick::ceil(vtime.get_value()) << " " << toString() << endl );
			status = RELEASING;
		}        
		DBGPRINT("Status is now " << status_string[status]);

		if (status == RELEASING) {
			EnergyMRTKernel* emrtk = energyMRTKernel();
			if (emrtk != NULL)
				emrtk->onExecutingReleasing(this); // schedule eventual ready task on same core
		}
	}

	void CBServerCallingEMRTKernel::idle_ready() {
		DBGENTER(_SERVER_DBG_LEV);
		assert (status == IDLE);
		status = READY;

		cap = 0;
		_jobLastCpu = NULL;

		if (idle_policy == REUSE_DLINE && SIMUL.getTime() < getDeadline()) {
			double diff = double(getDeadline() - SIMUL.getTime()) * 
			double(Q) / double(P);
			cap = Tick(std::floor(diff));
		}

		if (cap == 0) {
			cap = Q;

			//added relative deadline
			abs_dline = SIMUL.getTime() + P;
			DBGPRINT_2("new deadline ", abs_dline);
		}
		vtime.set_value(SIMUL.getTime());
		DBGPRINT_2("Going to active contending ",SIMUL.getTime());
	}

  	void CBServerCallingEMRTKernel::ready_executing()
	{
		ECOUTD( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "(), "  << toString() << ", _alreadyExecutedCycles=" << getAlreadyExecutedCycles() << endl );
		DBGENTER(_SERVER_DBG_LEV);

		status = EXECUTING;
		last_time = SIMUL.getTime();
		_jobLastCpu = energyMRTKernel()->getProcessorReadyRunning(this);
		_speed = getProcessor()->getSpeed();

		// vtime is incremented as in www.retis.sssup.it/~lipari/papers/lipariBaruah2001-1.pdf
		// dV_i/dt = 1/U_i if T_i is executing (and U_i changes with frequency in this case)
		// Tick wcet_nom = Tick::ceil(getWCET(CPU_BL::REFERENCE_SPEED));
		vtime.start( double(P) / (double(Q) / _speed) );
		
		_bandExEvt.post(last_time + Tick::ceil(double(cap) / _speed));
		ECOUT( "\tt=" << SIMUL.getTime() << " CBS::" << __func__ << "(), vtime=" << vtime.get_value() << " setting vtime with " << P << "/(" << Q << "/" << _speed << ")=" << (double) P / (double(Q) / _speed) << " for " << toString() );
		ECOUT( " - _bandExEvt posted at " << last_time << "+(" << cap << "/" << _speed << ")=" << last_time + Tick::ceil(double(cap) / _speed) << " for " << toString() << endl ); 
		DBGPRINT_2("Last time is: ", last_time);

		if (_pstrace != NULL) _pstrace->write(getFirstTask(), to_string(getProcessor()->getIndex()) + "\tRUNNING\tS");
	}


	/*The server is preempted. */
	void CBServerCallingEMRTKernel::executing_ready() {
		DBGENTER(_SERVER_DBG_LEV);

		status = READY;
		Tick lastExecutedCycles = Tick::ceil(double(SIMUL.getTime() - last_time) * _speed);
		cap = cap - lastExecutedCycles;
		_alreadyExecutedCycles += lastExecutedCycles;
		vtime.stop();
		_bandExEvt.drop();

		// cerr << toString() << " " << _speed << " " << _alreadyExecutedCycles << endl;
		// cerr << _alreadyExecutedCycles << " " << getFirstTask()->getWCET(1.0) << endl;
		EASSERT (double(_alreadyExecutedCycles) <= getFirstTask()->getWCET(1.0), this);
		ECOUT ("t=" << SIMUL.getTime() << ", " << toString() << " vtime=" << vtime.get_value() << " lastExecutedCycles=" << lastExecutedCycles << " _alreadyExecutedCycles=" << _alreadyExecutedCycles << " " << getProcessor()->toString() << " _speed=" << _speed << endl);

		if (_pstrace != NULL) _pstrace->write(getFirstTask(), to_string(getProcessor()->getIndex()) + "\tRUNNING\tE");
	}

	/// remember to call Server::setKernel(kern) before this
	void CBServerCallingEMRTKernel::releasing_idle() {
		ECOUTD( "t=" << SIMUL.getTime() << ", CBSCEMRTK::" << __func__ << "()" << endl );
		CBServer::releasing_idle();

		EnergyMRTKernel* emrtk = energyMRTKernel();
		if (emrtk != NULL) emrtk->onReleasingIdle(this); // forget U_active, migrate
	}

}
