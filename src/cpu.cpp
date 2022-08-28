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

#include <cpu.hpp>
#include <assert.h>
#include <energyMRTKernel.hpp>
#include <tracepowerbl.hpp>
#include <tracefreqbl.hpp>
#include <traceutilwlbl.hpp>

namespace RTSim
{

    unsigned int CPU_BL::REFERENCE_FREQUENCY;
    double       CPU_BL::REFERENCE_SPEED;

    string Island_BL::IslandName[NUM_ISLANDS] = {"BIG","LITTLE"};


    CPU::CPU(const string &name,
             const vector<double> &V,
             const vector<unsigned int> &F,
             CPUModel *pm) :

        Entity(name), frequencySwitching(0), index(0)
    {
        auto num_OPPs = V.size();

        cpuName = name;

        if (num_OPPs == 0) {
            PowerSaving = false;
            return;
        }

        PowerSaving = true;

        // Setting voltages and frequencies
        for (int i = 0; i < num_OPPs; i ++)
        {
            OPP opp;
            opp.voltage = V[i];
            opp.frequency = F[i];
            OPPs.push_back(opp);
        }

        // Creating the Energy Model class
        // and initialize it with the max values
        if (!pm) {
            powmod = new CPUModelMinimal(OPPs[currentOPP].voltage, OPPs[currentOPP].frequency);
            powmod->setFrequencyMax(OPPs[OPPs.size()-1].frequency);
        } else {
            powmod = pm;
            pm->setCPU(this);
        }

        /* Use the maximum OPP by default */
        currentOPP = num_OPPs - 1;

        _workload = "idle";

        // Setting speeds (basing upon frequencies)
        for (unsigned int opp = 0; opp < OPPs.size(); ++opp)
          OPPs[opp].speed = getSpeed(opp);
    }

    CPU::~CPU()
    {
        OPPs.clear();
    }

    int CPU::getOPP()
    {
        if (PowerSaving)
            return currentOPP;
        return 0;
    }

    void CPU::setOPP(unsigned int newOPP)
    {
        //std::ECOUT( __func__ << " setting currentOPP from " << currentOPP << " to " << newOPP << ", OPPs.size()=" << OPPs.size() << std::endl );
        assert(newOPP < OPPs.size() && newOPP >= 0);
        currentOPP = newOPP;
        updateCPUModel();
    }

    unsigned long int CPU::getFrequency() const
    {
        return getFrequency(currentOPP);
    }

    unsigned long int CPU::getFrequency(unsigned int opp) const {
        return OPPs[opp].frequency;
    }

    double CPU::getVoltage() const {
        return getVoltage(currentOPP);
    }

    double CPU::getVoltage(unsigned int opp) const {
        return OPPs[opp].voltage;
    }

    double CPU::getMaxPowerConsumption()
    {
        return _max_power_consumption;
    }

    void CPU::setMaxPowerConsumption(double max_p)
    {
        _max_power_consumption = max_p;
    }

    double CPU::getCurrentPowerConsumption()
    {
        if (PowerSaving) {
            updateCPUModel();
            return (powmod->getPower());
        }
        return 0.0;
    }

    double CPU::getCurrentPowerSaving()
    {
        if (PowerSaving)
        {
            long double maxPowerConsumption = getMaxPowerConsumption();
            long double saved = maxPowerConsumption - getCurrentPowerConsumption();
            return static_cast<double>(saved / maxPowerConsumption);
        }
        return 0;
    }

    double CPU::setSpeed(double newLoad)
    {
        DBGENTER(_KERNEL_DBG_LEV);
        DBGPRINT("pwr: setting speed in CPU::setSpeed()");
        DBGPRINT("pwr: New load is " << newLoad);
        if (PowerSaving)
        {
            DBGPRINT("pwr: PowerSaving=on");
            DBGPRINT("pwr: currentOPP=" << currentOPP);
            for (int i=0; i < (int) OPPs.size(); i++)
                if (OPPs[i].speed >= newLoad)
                {
                    if (i != currentOPP)
                        frequencySwitching++;
                    currentOPP = i;
                    DBGPRINT("pwr: New OPP=" << currentOPP <<" New Speed=" << OPPs[currentOPP].speed);

                    return OPPs[i].speed; //It returns the new speed
                }
        }
        else
            DBGPRINT("pwr: PowerSaving=off => Can't set a new speed!");

        return 1; // An error occurred or PowerSaving is not enabled
    }

    void CPU::setWorkload(const string &workload)
    {
	//if (workload == "idle")
	//	ECOUT( "\t" << getName() << " gets wl idle at t=" << SIMUL.getTime() << endl; );
        _workload = workload;
    }

    const string& CPU::getWorkload() const
    {
        return _workload;
    }
    
    double CPU::getSpeed()
    {
        if (PowerSaving) {
            updateCPUModel();
            return powmod->getSpeed();
        }
        return 1.0;
    }

    double CPU::getSpeed(unsigned int opp)
    {
        if (!PowerSaving)
            return 1;
        assert(opp < OPPs.size() && opp >= 0);
        int old_curr_opp = currentOPP;
        setOPP(opp);
        double s = getSpeed();
        setOPP(old_curr_opp);
        return s;
    }

    unsigned long int CPU::getFrequencySwitching()
    {
        DBGENTER(_KERNEL_DBG_LEV);
        DBGPRINT("frequencySwitching=" << frequencySwitching);

        return frequencySwitching;
    }

    string CPU::toString() {
        double freq = getFrequency();
        string res = getName() + " freq " + to_string(freq) + " (" + to_string(getSpeed()) + ")";
        return res;
    }

    void CPU::check()
    {
        ECOUT( "Checking CPU:" << cpuName << endl; );
        ECOUT( "Max Power Consumption is :" << getMaxPowerConsumption() << endl );
        for (vector<OPP>::iterator iter = OPPs.begin(); iter != OPPs.end(); iter++)
        {
            ECOUT( "-OPP-" << endl );
            ECOUT( "\tFrequency:" << (*iter).frequency << endl );
            ECOUT( "\tVoltage:" << (*iter).voltage << endl );
            ECOUT( "\tSpeed:" << (*iter).speed << endl );
        }
        for (unsigned int i = 0; i < OPPs.size(); i++)
            ECOUT( "Speed level" << getSpeed(i) << endl );
        for (vector<OPP>::iterator iter = OPPs.begin(); iter != OPPs.end(); iter++)
        {
            ECOUT( "Setting speed to " << (*iter).speed << endl );
            setSpeed((*iter).speed);
            ECOUT( "New speed is  " << getSpeed() << endl );
            ECOUT( "Current OPP is  " << getOPP() << endl );
            ECOUT( "Current Power Consumption is  " << getCurrentPowerConsumption() << endl );
            ECOUT( "Current Power Saving is  " << getCurrentPowerSaving() << endl );
        }
    }

    uniformCPUFactory::uniformCPUFactory()
    {
        _curr=0;
        _n=0;
        index = 0;
    }

    uniformCPUFactory::uniformCPUFactory(char* names[], int n)
    {
        _n=n;
        _names = new char*[n];
        for (int i=0; i<n; i++)
        {
            _names[i]=names[i];
        }
        _curr=0;
        index = 0;
    }

    CPU* uniformCPUFactory::createCPU(const string &name,
                                      const vector<double> &V,
                                      const vector<unsigned int> &F,
                                      CPUModel *pm)
    {
        CPU *c;

        if (_curr==_n)
            c = new CPU(name, V, F, pm);
        else
            c = new CPU(_names[_curr++], V, F, pm);

        c->setIndex(index++);
        return c;
    }

    // useless, because entity are copied and thus they change name by implementation of Entity...
    bool operator==(const CPU& lhs, const CPU& rhs) {
      return lhs.getName() == rhs.getName();
    }

    void CPU::updateCPUModel() {
        powmod->setVoltage(getVoltage());
        powmod->setFrequency(getFrequency());
    }

    // ------------------------------------------------------------- big little
    CPU_BL::CPU_BL(const string &name, const string &wl, CPUModel* powermodel) : CPU(name, {}, {}, NULL) {
        _isBusy = false;
        _workload = wl;
        _pm = powermodel;
        _disabled = false;
        assert(_pm != nullptr);
    }

    CPU_BL::~CPU_BL() {
      ECOUT( __func__ << endl );
      delete _pm;
    }

    void CPU_BL::setWorkload(const string& wl) {
      _workload = wl;
      _island->onFreqOrWlChanged(this);
      _island->onUtilOrWlChanged();
   }

    unsigned long int CPU_BL::getFrequency() const {
        return _island->getFrequency();
    }

    unsigned long int CPU_BL::getFrequency(unsigned int opp) const {
        return _island->getFrequency(opp);
    }

    double CPU_BL::getVoltage() const {
        return _island->getVoltage();
    }
    
    double CPU_BL::getVoltage(unsigned int opp) const {
        return _island->getVoltage(opp);
    }

    unsigned int CPU_BL::getOPP() const {
        return _island->getOPP();
    }

    void CPU_BL::setOPP(unsigned int opp, bool notifyKernel) {
        _island->setOPP(opp, notifyKernel);
    }

    vector<struct OPP> CPU_BL::getHigherOPPs() {
        return _island->getHigherOPPs();
    }

    void CPU_BL::setBusy(bool busy) {
        _isBusy = busy;
        getIsland()->notifyBusy(this);
    }

    bool CPU_BL::isIslandBusy() {
        return _island->isBusy();
    }

    bool CPU_BL::hasMaxUtilization() const {
        bool isMax = (getIsland()->getMaxUtilization().begin()->second == this);
        return isMax;
    }

    bool CPU_BL::hasMinUtilization() const {
        bool isMin = (getIsland()->getCoreWithLowestUtilization().cpu == this);
        return isMin;
    }

    // expects amount to be nominal, scaled with nothing!
    void CPU_BL::increaseUtilReadyRunning(double amount) { 
        double init = _totalUtil_cur;
        _totalUtil_cur += amount;
        assert(amount >= 0.0); assert (init >= -0.00000001); assert (_totalUtil_cur >= -0.00000001); assert (amount >= 0.0);
        if (!getIsland()->getKernel()->isGEDF())
            assert(_totalUtil_cur <= 1.0);
        _island->notifyLoadChange(init, _totalUtil_cur, getUtilization_active(), getUtilization_active(), this);
    }

    void CPU_BL::decreaseTotalNomUtil(double amount, bool adjustFreq) {
        double init = _totalUtil_cur;
        _totalUtil_cur -= amount;
        assert(amount >= 0.0); assert (init >= -0.00000001); assert (_totalUtil_cur >= -0.00000001); assert (amount >= 0.0);
        if (!getIsland()->getKernel()->isGEDF())
            assert(_totalUtil_cur <= 1.0); 
        _island->notifyLoadChange(init, _totalUtil_cur, getUtilization_active(), getUtilization_active(), this, adjustFreq);
    }

    /// call it before to update the kernel map
    void CPU_BL::increaseUtilActive(double amount) {
        double init = getUtilization_active();
        double newUactive = init + amount;
        _active_util = newUactive;

        assert (init >= -0.00000001); assert (_active_util >= -0.00000001); assert (amount >= 0.0);
        if (!getIsland()->getKernel()->isGEDF())
            assert (_active_util <= 1.0);

        _island->notifyLoadChange(_totalUtil_cur, _totalUtil_cur, init, _active_util, this);
    }

    /// call it before to update the EMRTKernel map
    void CPU_BL::decreaseUtilActive(double amount, bool adjustFreq) {
        double init = getUtilization_active();
        double newUactive = init - amount;
        _active_util = newUactive;
        if (_active_util < 0.0)  // policy of forgetting active util when core get idle makes me introduce this check
            _active_util = 0.0;

        assert (init >= -0.00000001); assert (_active_util >= -0.00000001); assert (amount >= 0.0);
        if (!getIsland()->getKernel()->isGEDF())
            assert(_active_util <= 1.0);

        _island->notifyLoadChange(_totalUtil_cur, _totalUtil_cur, init, _active_util, this, adjustFreq);
    }

    IslandType CPU_BL::getIslandType() const {
        return _island->getIslandType();
    }

    double CPU_BL::getCurrentPowerConsumption()
    {
        updateCPUModel(getOPP());
        return (_pm->getPower());
    }

    double CPU_BL::getPowerConsumption(double frequency) {
        // Find what OPP corresponds to provided frequency
        assert (frequency >= _island->getOPPs().at(0).frequency);
        unsigned int old_opp = getOPP();
        unsigned int opp = _island->getOPPByFrequency(frequency);

        updateCPUModel(opp);
        double pow = _pm->getPower();
        updateCPUModel(old_opp);

        return pow;
    }

    double CPU_BL::getSpeed(double freq) {
        assert (freq >= _island->getOPPs().at(0).frequency);
        unsigned int opp = _island->getOPPByFrequency(freq);
        return getSpeed(opp);
    }

    double CPU_BL::getSpeed(unsigned int opp) {
        int old_curr_opp = getOPP();

        updateCPUModel(opp);
        double s = _pm->getSpeed();
        updateCPUModel(old_curr_opp);
        return s;
    }

    vector<OPP> CPU_BL::getOPPs() const {
        return _island->getOPPs(); 
    }


    double CPU_BL::getSpeed() {
        assert(getOPP() < _island->getOPPsize() && getOPP() >= 0);
        updateCPUModel(getOPP());
        return _pm->getSpeed();
    }

    double CPU_BL::getUtilization_active() const {
        double res = _active_util;
        return res;
    }

    CPU_BL* CPU_BL::forgetUtilization_active() {
        ECOUT( "\tt=" << SIMUL.getTime() << ", CPU_BL::" << __func__ << "() (forget all active util) on " << getName() << endl );
        decreaseUtilActive(getUtilization_active(), false); // don't adjust frequency
        _tasksWithSavedUtilization_active.erase(_tasksWithSavedUtilization_active.begin(), _tasksWithSavedUtilization_active.end());
        return this;
    }

    CPU_BL* CPU_BL::forgetUtilization_active(CBServer* cbs) {
        ECOUT( "\tt=" << SIMUL.getTime() << ", CPU_BL::" << __func__ << "() for " << cbs->toString() << " on " << getName() << endl );
        
        double jobActiveUtil = dynamic_cast<CBServerCallingEMRTKernel*>(cbs)->getUtilization(); // server/task current utilization, not scaled
        
        // check if we have already forgotten task active utilization ( case saveUactive(task), core gets free => fU_active(), fU_active(task) )
        for (int i = 0; i < _tasksWithSavedUtilization_active.size(); i++)
            if (_tasksWithSavedUtilization_active.at(i) == cbs) {
                decreaseUtilActive(jobActiveUtil, false); // don't adjust frequency
                _tasksWithSavedUtilization_active.erase(_tasksWithSavedUtilization_active.begin() + i);
                break;
            }

        ECOUT( "\tt=" << SIMUL.getTime() << ", total active util on " << getName() << " is " << getUtilization_active() << endl );

        return this;
    }

    void CPU_BL::saveUtilization_active(CBServer* cbs) {
        ECOUT( "CPU_BL::" << __func__ << "() " << " for " << cbs->toString() << " on " << getName() << ", ends at t=" << Tick::ceil(cbs->getVirtualTime()) << endl );
        assert(cbs != NULL);

        string originalWL = getWorkload();
        setWorkload("bzip2");
          
        double speed = getSpeed();
        // onEnd(): island gets empty; onExecutingReleasing() comes after, but freq == 200
        if (!getIsland()->isBusy()) {
          unsigned int prevOPP = getIsland()->getOPPBeforeIslandFree();
          assert (prevOPP > 0);
          speed = getSpeed(prevOPP);
          ECOUT( "t=" << SIMUL.getTime() << __func__ << "(). Detected free island " << getName() << ", speed before free was " << speed << endl ); 
        }
        assert (speed > 0.0 && speed <= CPU_BL::REFERENCE_SPEED);

        // you cannot read cbs->getEndOfVirtualTime() because CBS::_idleEvt is posted in a CBS event called afterwards!
        Tick vt = Tick(cbs->getVirtualTime()); // tasks are status=EXECUTING
        if ( double(vt) < double(SIMUL.getTime()) )
            ECOUT( "\tt=" << SIMUL.getTime() << ", vt = " << vt << " in the past for " << cbs->toString() << " on " << getName() << " => skip. idle evt=" << cbs->getIdleEvent() << endl );
        else {
            double u_active_task = dynamic_cast<CBServerCallingEMRTKernel*>(cbs)->getUtilization(); // server/task current utilization, not scaled
            increaseUtilActive(u_active_task);
            _tasksWithSavedUtilization_active.push_back(cbs);

            // a better map is by cpu, but then cpus can collide. The map's used for traceability of migrations
            ECOUT( "\tt=" << SIMUL.getTime() << ", total active util on " << getName() << " is " << getUtilization_active() << " (added " << cbs->getName() << " -> " << u_active_task << ", canceling at t=" << vt << ")" << endl );
        }
        
        setWorkload(originalWL);
    }

    void CPU_BL::setMaxFrequency() {
        setOPP(_island->getOPPsize() - 1, false); // should be true, but rush..
    }

    // could be removed at all in favour of uCPUM(newOPP)
    void CPU_BL::updateCPUModel() {
        _pm->setVoltage(getVoltage());
        _pm->setFrequency(getFrequency());

        _island->onFreqOrWlChanged(this);
    }

    void CPU_BL::updateCPUModel(unsigned int opp) {
        // wrt updateCPUModel(), this one does not update physically the opp, just reads the data pretending to change opp
        _pm->setVoltage(getVoltage(opp));
        _pm->setFrequency(getFrequency(opp));
    }

    void CPU_BL::toggleDisabled() {
        _disabled = !_disabled;
        _island->notifyToggleDisabled(this);
    }

    string CPU_BL::pcu() const { return _island->pcu(); }

    Island_BL::Island_BL(const string &name, const IslandType island, const vector<CPU_BL *> cpus,
            const vector<struct OPP> opps)
            : Entity(name) {
        _island     = island;
        _cpus       = cpus;
        _opps       = opps;
        _currentOPP = 0;
        _oppBeforeIslandFree = 0;
        _nFreqIncreases = 0;
        _maxUtil = 0.0;

        for (CPU_BL* c : _cpus)
            c->_island = this;

        string filename = UTIL_POW_BIG;
        ifstream fileexist1(filename);
        assert (fileexist1.good());
        if (island == IslandType::LITTLE) filename = UTIL_POW_LITTLE;
        ifstream file(filename);
        map<double, double> graphUtilPower = jsonToMap( string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()) );
        _asVector(graphUtilPower, true);

        filename = UTIL_FREQ_BIG;
        ifstream fileexist2(filename);
        assert (fileexist2.good());
        if (island == IslandType::LITTLE) filename = UTIL_FREQ_LITTLE;
        ifstream file1(filename);
        map<double, double> graphUtilFreq = jsonToMap( string((istreambuf_iterator<char>(file1)), istreambuf_iterator<char>()) );
        _asVector(graphUtilFreq, false);

        UtilCorePair* cc = new UtilCorePair[_cpus.size()];
        for(int i = 0; i < _cpus.size(); i++)
            cc[i] = UtilCorePair(0.0, _cpus[i]);
        _coresUtil = new MinHeap<UtilCorePair>(cc, 4, 4);

        assert(!opps.empty() && !cpus.empty());
        assert(_island == IslandType::BIG || _island == IslandType::LITTLE);
    };

    Island_BL::~Island_BL() {
        ECOUT( __func__ << endl );
        _opps.clear();
        for (CPU_BL* c : _cpus)
          delete c;
        _cpus.clear();
        delete _coresUtil;

        if (_powerTracer != NULL)
            delete _powerTracer;

        if (_utilTracer != NULL)
            delete _utilTracer;
    }

    void Island_BL::storeCurFrequency()
    {
        _freqTracer->onFreqChanged(true);
    }

    void Island_BL::setSynchFreqTracerDirectory(const string& dirFreqTracer) {
        if (dirFreqTracer != "") {
            delete _freqTracer;
            _freqTracer = new TraceFreqBLSynch(this, dirFreqTracer + "freq" + (getIslandType() == IslandType::BIG ? "BIG" : "LITTLE") + ".txt");
        }
    }

    void Island_BL::setSynchPowerTracerDirectory(const string &dirPowTracer)
    {
        if (dirPowTracer != "")
        {
            delete _powerTracer;
            _powerTracer = new TracePowerConsumptionSynch(this, dirPowTracer + "power" + (getIslandType() == IslandType::BIG ? "BIG" : "LITTLE") + ".txt");
        }
    }

    void Island_BL::setSynchUtilWlTracerDirectory(const string &dirUtilWlTracer)
    {
        if (dirUtilWlTracer != "")
        {
            delete _utilTracer;
            _utilTracer = new TraceUtilWlSynch(this, dirUtilWlTracer + "util_wl" + (getIslandType() == IslandType::BIG ? "BIG" : "LITTLE") + ".txt");
        }
    }

    void Island_BL::onFreqOrWlChanged(CPU_BL *cpu)
    {
        if (_powerTracer != NULL)
            _powerTracer->onFreqOrWlChanged();
    }

    void Island_BL::onUtilOrWlChanged()
    {
        if (_utilTracer != NULL)
            _utilTracer->onUtilOrWlChanged();
    }

    void Island_BL::_asVector(map<double,double> map, bool doPowers) {
        _nomUtil_steps_no = map.size() - 1;
        const double delta_u = 1.0 / _nomUtil_steps_no;
        for (int uid = 0; uid < _nomUtil_steps_no; ++uid) {
            const double pos = uid*delta_u;
            auto it = map.upper_bound(pos);
            assert (it != map.end());
            double val = it->second;
            if (doPowers)
                _graphUtilPower.push_back(val);
            else
                _graphUtilFreq.push_back(val);
        }

        // check validity
        if (doPowers)
            for (int uid = 0; uid < _graphUtilPower.size() - 1 && _graphUtilPower[uid + 1] != -1; uid++)
                assert (_graphUtilPower[uid] <= _graphUtilPower[uid + 1]);
        else
            for (int uid = 0; uid < _graphUtilFreq.size() - 1 && _graphUtilFreq[uid + 1] != -1; uid++)
                assert (_graphUtilFreq[uid] <= _graphUtilFreq[uid + 1]);
    }

    UtilCorePair Island_BL::getCoreWithLowestUtilization(const vector<CPU_BL*> &toBeExcluded) const {
        UtilCorePair pair = _coresUtil->get_min();
        return pair;
    }

    double Island_BL::getPowerConsFromGraph(double util) const {
        static const double delta_u = 1.0 / _nomUtil_steps_no; // a pick on the graph
        const int uid = ceil(util / delta_u);
        assert ( uid < _graphUtilPower.size() );
        double pow = _graphUtilPower[uid];
        return pow;
    }

    double Island_BL::getFrequencyFromGraph(double util) const {
        if (getKernel()->isGEDF() && _island == IslandType::LITTLE && util > getUtilizationLimit())
            return 1400; // max freq

        if (_island == IslandType::LITTLE && util > getUtilizationLimit() )
            return -1.0; // no abort() because who calls me checks my results

        static const double delta_u = 1.0 / _nomUtil_steps_no; // a pick on the graph
        const int uid = ceil(util / delta_u);
        double freq = 0.0;

        if ( uid >= _graphUtilFreq.size() )
            freq = _graphUtilFreq.back();
        else
            freq = _graphUtilFreq[uid];
        
        assert (freq > 0.0);
        return freq;
    }

    void Island_BL::setOPP(unsigned int opp, bool notifyKernel) {
        assert(opp >= 0 && opp < _opps.size());
        static int64_t lastTimeFreqIncreased = -1;  // last time frequency increased. Declared static here because only used here
        unsigned int oldOPP = _currentOPP;
        unsigned int newOPP = opp;
        _currentOPP = opp;

        // time check needed: e.g., task arrives -> f=400; other task arrives at same time -> f=800
        if (oldOPP < newOPP && int64_t(SIMUL.getTime()) > lastTimeFreqIncreased) {
            _nFreqIncreases++;
            lastTimeFreqIncreased = int64_t(SIMUL.getTime());
        }

        for (CPU_BL* c : getProcessors())
            c->updateCPUModel();
        if (notifyKernel)
            _kernel->onOppChanged(oldOPP, newOPP, this);
    }

    void Island_BL::notifyToggleDisabled(CPU_BL* cpu) {
        if (cpu->isDisabled()) {
            _disabledCPUs.push_back(cpu);
        }
        else
            for (int i = 0; i < _disabledCPUs.size(); i++)
                if (cpu == _disabledCPUs.at(i))
                    _disabledCPUs.erase(_disabledCPUs.begin() + i);
    }

    void Island_BL::notifyLoadChange(double fromUtil, double toUtil, double fromU_active, double toU_active, CPU_BL* cpu, bool adjustFreq) {
        ECOUT ( "t=" << SIMUL.getTime() << ", Island_BL::" << __func__ << "() for " << cpu->toString() << " total util from " << fromUtil + fromU_active << " to " << toUtil + toU_active << endl );
        UtilCorePair newElem = UtilCorePair(toUtil + toU_active, cpu);
        UtilCorePair oldElem = UtilCorePair(fromUtil + fromU_active, cpu);

        _coresUtil->remove(oldElem);
        _coresUtil->insert(newElem);
        assert (_coresUtil->getSize() == 4);


        #ifdef DEBUG
        for (int i = 0; i < 4; i++)
            if (!getKernel()->isGEDF())
                assert(_coresUtil->show_element(i).util <= 1.0);
        #endif

        _maxUtil = _coresUtil->get_max().util; // speed
        
        ECOUT( pcu() << "max " << getName() << "=" << _maxUtil << endl );

        if (adjustFreq)
            adjustFrequency();

        onUtilOrWlChanged();
    }

    void Island_BL::adjustFrequency() {
        ECOUT ( "t=" << SIMUL.getTime() << ", Island_BL::" << __func__ << "() " << getName() << endl );
        _maxUtil = _coresUtil->get_max().util; // speed. _maxUtil is the util of running and ready tasks only
        double freq = getFrequencyFromGraph(_maxUtil); // util ready+runnning+active
        unsigned int opp = getOPPByFrequency(freq);
        setOPP(opp);

        _freqTracer->onFreqChanged();
        ECOUT("\tisland " << getName() << " has new frequency " << getFrequency() << endl);
    }

}
