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
#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <config.hpp>

#include <set>
#include <string>
#include <vector>

#include <trace.hpp>
#include <timer.hpp>
#include <powermodel.hpp>
#include <abstask.hpp>
#include <assert.h>

#include <minheap.hpp>
#include <unistd.h>

// #include <tracefreqbl.hpp>
// #include <tracepower.hpp>

#define _KERNEL_DBG_LEV "Kernel"

#define CUR_PATH         string(".")
#define UTIL_POW_BIG     CUR_PATH + "/data/util_power_bzip2_big.dat"
#define UTIL_POW_LITTLE  CUR_PATH + "/data/util_power_bzip2_little.dat"
#define UTIL_FREQ_BIG    CUR_PATH + "/data/util_freq_bzip2_big.dat"
#define UTIL_FREQ_LITTLE CUR_PATH + "/data/util_freq_bzip2_little.dat"

namespace RTSim
{

    using namespace std;
    using namespace MetaSim;

    class EnergyMRTKernel;
    class TraceFreqBLSynch;
    class TracePowerConsumptionSynch;
    class TraceUtilWlSynch;
    class CBServer;

    struct OPP {
        /// Voltage of each step (in Volts)
        double voltage;

        /// Frequency of each step (in MHz)
        unsigned int frequency;

        /// The speed is a value between 0 and 1
        double speed;
    };

    /**
     * \ingroup kernels
     *
     * A CPU doesn't know anything about who's running on it: it just has a
     * speed factor. This model contains the energy values (i.e. Voltage and
     * Frequency) of each step. The speed of each step is calculated basing
     * upon the step frequencies.  The function setSpeed(load) adjusts the CPU
     * speed accordingly to the system load, and returns the new CPU speed.
     */
    class CPU : public Entity {
    protected:
	/// Delta workload
	string _workload;
    private:
        double _max_power_consumption;

        /**
             *  Energy model of the CPU
             */
        CPUModel *powmod;

        vector<OPP> OPPs;

        /// Name of the CPU
        string cpuName;

        /// currentOPP is a value between 0 and OPPs.size() - 1
        unsigned int currentOPP;

        bool PowerSaving;

        /// Number of speed changes
        unsigned long int frequencySwitching;

        // this is the CPU index in a multiprocessor environment
        int index;

        /// update CPU power/speed model according to currentOPP
        virtual void updateCPUModel();


    public:

        /// Constructor for CPUs
        CPU(const string &name="",
            const vector<double> &V= {},
            const vector<unsigned int> &F= {},
            CPUModel *pm = nullptr);

        virtual ~CPU();

        virtual string getName() const {
            return Entity::getName();
        }

        virtual string toString();

        /// set the processor index
        void setIndex(int i)
        {
            index = i;
        }

        /// get the processor index
        inline int getIndex() const
        {
            return index;
        }

        virtual vector<OPP> getOPPs() const { return OPPs; }

        /// Useful for debug
        virtual int getOPP();

        /// Useful for debug
        virtual void setOPP(unsigned int newOPP);

        virtual unsigned long int getFrequency() const;

        virtual unsigned long int getFrequency(unsigned int opp) const;

        virtual double getVoltage() const;

        virtual double getVoltage(unsigned int opp) const;


        /// Returns the maximum power consumption obtainable with this
        /// CPU
        virtual double getMaxPowerConsumption();

        /// Returns the maximum power consumption obtainable with this
        /// CPU
        virtual void setMaxPowerConsumption(double max_p);

        /// Returns the current power consumption of the CPU If you
        /// need a normalized value between 0 and 1, you should divide
        /// this value using the getMaxPowerConsumption() function.
        virtual double getCurrentPowerConsumption();

        /// Returns the current power saving of the CPU
        virtual double getCurrentPowerSaving();

        /** Sets a new speed for the CPU accordingly to the system
         *  load.  Returns the new speed.
         */
        virtual double setSpeed(double newLoad);

        /**
         * Set the computation workload on the cpu
         */
        virtual void setWorkload(const string &workload);

        virtual const string& getWorkload() const;

        /// Returns the current CPU speed (between 0 and 1)
        virtual double getSpeed();

        virtual double getSpeed(unsigned int opp);

        virtual unsigned long int getFrequencySwitching();

        /// Needed for CPU_BL, but makes sense for CPU too
        virtual void increaseUtilReadyRunning(double amount) {}

        /// Needed for CPU_BL, but makes sense for CPU too
        virtual void decreaseTotalNomUtil(double amount, bool notifyIsland = true) {}

        virtual void newRun() {}
        virtual void endRun() {}

        ///Useful for debug
        virtual void check();
    };

    bool operator==(const CPU& rhs, const CPU& lhs);
    
    /**
     * The abstract CPU factory. Is the base class for every CPU factory which
     * will be implemented.
     */
    class absCPUFactory {

    public:
        virtual CPU* createCPU(const string &name="",
                               const vector<double> &V= {},
                               const vector<unsigned int> &F= {},
                               CPUModel *pm = nullptr) = 0;
        virtual ~absCPUFactory() {}
    };


    /**
     * uniformCPUFactory. A factory of uniform CPUs (whose speeds are maximum).
     * Allocates a CPU and returns a pointer to it
     */
    class uniformCPUFactory : public absCPUFactory {

        char** _names;
        int _curr;
        int _n;
        int index;
    public:
        uniformCPUFactory();
        uniformCPUFactory(char* names[], int n);
        /*
             * Allocates a CPU and returns a pointer to it
             */
        CPU* createCPU(const string &name="",
                       const vector<double> &V= {},
                       const vector<unsigned int> &F= {},
                       CPUModel *pm = nullptr);
    };

    /**
     * Stores already created CPUs and returns the pointers, one by one, to the
     * requesting class.
     */
    class customCPUFactory : public absCPUFactory {

        list<CPU *> CPUs;

    public:

        customCPUFactory() {}

        void addCPU(CPU *c)
        {
            CPUs.push_back(c);
        }

        /*
             * Returns the pointer to one of the stored pre-allocated CPUs.
             */
        CPU *createCPU(const string &name="",
                       const vector<double> &V= {},
                       const vector<unsigned int> &F= {},
                       CPUModel *pm = nullptr)
        {
            if (CPUs.size() > 0) {
                CPU *ret = CPUs.front();
                CPUs.pop_front();
                return ret;
            }
            return nullptr;
        }
    };

  // ------------------------------------------------------------ Big-Little. Classes not meant to be extended
  // Design: I had a class "CPU", which already seemed ok for managing Big-Littles. However, this brought bugs,
  // for example when updating an OPP, since it didn't update automatically the OPP of the island.
  // With CPU_BL, instead, a core belongs to an IslandType and changing OPP means changing IslandType OPP. Moreover, classes were
  // built while realizing EnergyMRTKernel, which is a kinda self-adaptive kernel and thus needs to try different
  // OPPs to dispatch a task. Thus, those 2 concepts are tight. One aim was not to break the existing code written by others and
  // make their experiments work => don't touch others classes, or limit modifications.
  // It seems I'm breaking Liskov Substitution Principle. Hope not too much... :) 

  class Island_BL;
  class AbsRTTask;
  typedef enum { LITTLE=0, BIG, NUM_ISLANDS } IslandType;




  class CPU_BL final : public CPU {
      friend class Island_BL; // otherwise I would have infinite recursion
  private:
    Island_BL* _island;

    /// Is CPU holding a task, either running and ready (= dispatching)?
    bool _isBusy;

    // pm belongs to CPU_BL and not to Island_BL because CPUs may execute different workloads
    CPUModel* _pm;

    /// for debug. CPU can be disabled, so that the scheduler won't dispatch any new task (but the forced ones)
    bool _disabled;

    /// Core total utilization (i.e., referred to big core at max speed) of ready and running tasks. It's already / REF_SPEED
    /// Does not include active utilization
    double _totalUtil_cur = 0.0;

    /// CPU current active utilization
    double _active_util = 0.0;

    /// list of tasks for which the CPU is keeping their active utilizations
    vector<AbsRTTask*> _tasksWithSavedUtilization_active;

    virtual void updateCPUModel();

  public:
    /// Reference CPUs frequency to compute CPU capacity. It is a global field to all CPUs
    static unsigned int REFERENCE_FREQUENCY;

    /// Reference CPUs speed. It is a global field to all CPUs
    static double REFERENCE_SPEED;

    CPU_BL(const string &name = "", const string &wl = "idle", CPUModel *powermodel = nullptr);

    virtual ~CPU_BL();

    virtual void setWorkload(const string &wl);

    virtual unsigned int getOPP() const;

    virtual vector<OPP> getOPPs() const;

    virtual void setOPP(unsigned int opp, bool notifyKernel = true);

    vector<struct OPP> getHigherOPPs();

    inline vector<AbsRTTask*> getTasksWithUtilization_active() const { return _tasksWithSavedUtilization_active; }

    inline double getUtilizationReadyRunning() const { return _totalUtil_cur; }


    // -------------------------- active utilization
    double getUtilization_active() const;

    CPU_BL* forgetUtilization_active();

    CPU_BL* forgetUtilization_active(CBServer* cbs);

    void saveUtilization_active(CBServer* cbs);



    bool hasMaxUtilization() const;

    bool hasMinUtilization() const;

    void setBusy(bool busy);

    bool isIslandBusy();

    inline bool isBusy() const {
        return _isBusy;
    }

    /// for debug. Tells if CPU is disabled, i.e. accepts new dispatched tasks
    inline bool isDisabled() const { return _disabled; }

    /// It also adjusts the island frequency
    virtual void increaseUtilReadyRunning(double amount);

    /// It also adjusts the island frequency if adjustFreq=true
    virtual void decreaseTotalNomUtil(double amount, bool adjustFreq = true);

    virtual void increaseUtilActive(double amount);

    virtual void decreaseUtilActive(double amount, bool adjustFreq = true);

    /// Enables or disables CPU
    void toggleDisabled();

    inline Island_BL* getIsland() const { 
        return _island;
    }

    IslandType getIslandType() const;

    /// Tells if this core is a big one
    inline bool isBig() const { return getIslandType() == IslandType::BIG; }

    virtual double getCurrentPowerConsumption();

    double getPowerConsumption(double frequency);

    virtual double getSpeed();

    double getSpeed(double freq);

    virtual double getSpeed(unsigned int opp);

    virtual unsigned long int getFrequency() const;

    virtual unsigned long int getFrequency(unsigned int opp) const;

    virtual double getVoltage() const;

    virtual double getVoltage(unsigned int opp) const;

    void setMaxFrequency();
    
    void updateCPUModel(unsigned int opp);

    string pcu() const;
  };



    class UtilCorePair { 
    public: 
        double util;
        CPU_BL* cpu;

        UtilCorePair() {}

        UtilCorePair(double u, CPU_BL* c) {
            util = u;
            cpu  = c;
        }

        bool operator<(const UtilCorePair& rhs) { 
            return this->util < rhs.util;
        }

        bool operator>(const UtilCorePair& rhs) {
            return this->util > rhs.util;
        }

        bool operator==(const UtilCorePair& rhs) {
            return this->util == rhs.util && this->cpu == rhs.cpu;
        }

        string toString() const {
            return to_string(util) + ", " + cpu->getName();
        }
    };

  class Island_BL final : public Entity {
  public:
    static string IslandName[NUM_ISLANDS];

  private:
    IslandType _island;

    vector<CPU_BL*> _cpus;

    vector<struct OPP> _opps;

    unsigned int _currentOPP;

    unsigned int _oppBeforeIslandFree;

    EnergyMRTKernel* _kernel;

    /// # frequency increases of island of the whole simulation
    unsigned int _nFreqIncreases;

    // ----------------- Additions for efficient implementation

    /// disabled CPUs
    vector<CPU_BL*> _disabledCPUs;

    /// graph nominal util -> power consumption
    vector<double> _graphUtilPower;

    /// graph nominal util -> freq
    vector<double> _graphUtilFreq;

    /// # steps used to pick the graph util -> pow
    unsigned int _nomUtil_steps_no = 0;

    /// Total nominal utilizations for each core of the island (min heap)
    MinHeap<UtilCorePair> *_coresUtil;

    /// Total nominal utilization of the most loaded core of this island. Used to search in RB tree
    double _maxUtil;

    /// threshold after which it's convenient not to use LITTLE island. See paper
    const double _utilizationLimit = 0.34532;

    /// frequency tracer. You need to reduce disk accesses
    TraceFreqBLSynch* _freqTracer = NULL;

    /// power tracer. You need to reduce disk accesses
    TracePowerConsumptionSynch *_powerTracer = NULL;

    /// util tracer. You need to reduce disk accesses
    TraceUtilWlSynch *_utilTracer = NULL;

public:
    Island_BL(const string &name, const IslandType island, const vector<CPU_BL *> cpus,
            const vector<struct OPP> opps);

    ~Island_BL();

    // static to oblige you to call this method first
    static vector<struct OPP> buildOPPs(const vector<double> &V= {},
                                        const vector<unsigned int> &F= {}) {
        assert(V.size() > 0 && V.size() == F.size());
        vector<struct OPP> OPPs;
        for (int i = 0; i < V.size(); i ++)
        {
            struct OPP opp;
            opp.voltage = V[i];
            opp.frequency = F[i];
            OPPs.push_back(opp);
        }
        return OPPs;
    }

    static map<double, double> jsonToMap(string json) {
        // I expect like json = { "0.12": 0.2131, "0.22": -1 }
        json.erase(json.begin(), json.begin()+1);
        json.erase(json.end()-1, json.end());

        istringstream iss(json);
        string util, watt;
        map<double, double> result; // nominal util -> watt

        while (iss >> util >> watt) { 
            util.erase(util.begin(), util.begin()+1);
            util.erase(util.end()-2, util.end());
            if (watt.back() == ',')
                watt.erase(watt.end()-1, watt.end());
            result[stod(util)] = stod(watt);
        }

        return result;
    }

    void storeCurFrequency();

    void setSynchFreqTracerDirectory(const string& dirFreqTracer="");

    void setSynchPowerTracerDirectory(const string &dirPowTracer = "");

    void setSynchUtilWlTracerDirectory(const string &dirUtilWlTracer = "");

    void onFreqOrWlChanged(CPU_BL* cpu);

    void onUtilOrWlChanged();

    /// Turns the map<util_nom, freq/pow> into a vector, to fasten the algorithm
    void _asVector(map<double,double> map, bool doPowers);

    inline vector<CPU_BL*> getDisabledCPUs() const {
        return _disabledCPUs;
    }

    /// Retuns a vector of cpus ordered by their utilizations
    inline MinHeap<UtilCorePair>* getCoresUtilPair() const { return _coresUtil; }

    inline unsigned int getFrequencyIncreases() const { return _nFreqIncreases; }

    inline EnergyMRTKernel* getKernel() const { return _kernel; }

    inline unsigned int getOPPsize() const { return _opps.size(); }

    inline unsigned int getIslandIndex() const { return getIslandType(); }

    inline IslandType getIslandType() const { return _island; }

    inline string getIslandTypeString() const { if (_island == IslandType::LITTLE) return "little"; else return "big"; }

    inline vector<CPU_BL*> getProcessors() const { return _cpus; }

    /// Returns the utilization limit after which it's more convenient not to choose LITTLE island - efficient impl
    inline double getUtilizationLimit() const { if (_island == IslandType::LITTLE) return _utilizationLimit; else return 1.0; }

    inline double getVoltage() const {
        return getStructOPP(getOPP()).voltage;
    }

    /// Returns the core with min utilization of this island and the util - efficient impl
    UtilCorePair getCoreWithLowestUtilization(const vector<CPU_BL*>& toBeExcluded = {}) const;

    /// Returns the core with max utilization of this island and the util - efficient impl
    map<double, CPU_BL*> getMaxUtilization() const {
        map<double, CPU_BL*> m;
        UtilCorePair* it = _coresUtil->find(_maxUtil);
        assert (it != NULL);
        m[_maxUtil] = it->cpu;
        return m; // you actually only need the utilization for the alg.
    }

    double getPowerConsFromGraph(double util) const;

    double getFrequencyFromGraph(double util) const;

    inline unsigned long int getFrequency(unsigned int opp) const {
        return getStructOPP(opp).frequency;
    }

    inline unsigned long int getFrequency() const {
        return getStructOPP(getOPP()).frequency;
    }

    inline double getVoltage(unsigned int opp) const {
        return getStructOPP(opp).voltage;
    }

    inline unsigned int getOPP() const {
        return _currentOPP;
    }

    inline vector<OPP> getOPPs() const {
        return _opps;
    }

    inline unsigned int getOPPBeforeIslandFree() const { return _oppBeforeIslandFree; }

    void setOPP(unsigned int opp, bool notifyKernel = true);

    vector<struct OPP> getHigherOPPs() {
        int maxOPP = _currentOPP;
        assert(_currentOPP >= 0 && _currentOPP < _opps.size());
        vector<struct OPP> opps;
        for (int i = maxOPP; i < _opps.size(); i++) {
            opps.push_back(_opps.at(i));
        }
        return opps;
    }

    bool isBusy() {
        for (CPU_BL* c : _cpus)
            if (c->isBusy()){
                // ECOUT( c->toString() << " is busy"<<endl );
                return true;
            }
        return false;
    }

    unsigned int getOPPByFrequency(double frequency) const {
        assert(frequency >= _opps.at(0).frequency);
        for (int i = 0; i < _opps.size(); i++)
            if (_opps.at(i).frequency == frequency)
                return i;
        // exception...
        abort();
        return -1;
    }

    inline struct OPP getMinOPP() const {
        return getStructOPP(0);
    }

    inline struct OPP getStructOPP(int i) const {
        assert(i >= 0 && i < _opps.size());
        return _opps.at(i);
    }

    inline struct OPP getStructOPP() const {
        return getStructOPP(getOPP());
    }

    unsigned int getOPPindex(struct OPP opp) const {
        return getOPPByFrequency(opp.frequency);
    }

    /// Notifies the island that one of its cores've got free
    void notifyBusy(CPU_BL* cpu) {
        if (!this->isBusy()) // if island is free
            _oppBeforeIslandFree = getOPP();
    }

    void notifyToggleDisabled(CPU_BL* cpu);

    /// CPU_BL notifies that its load has changed, thus island should set its speed
    void notifyLoadChange(double fromUtil, double toUtil, double fromU_active, double toU_active, CPU_BL* cpu, bool adjustFrequency=true);

    /// Adjusts the frequency according to the most loaded core
    void adjustFrequency();

    void setKernel(EnergyMRTKernel* e) { _kernel = e; }

    string toString() {
      return getName();
    }

    /// for debug
    string pcu() {
        return getCoresUtilPair()->toString();
    }

    virtual void newRun() {}
    virtual void endRun() {}

    // functions of base class you shouldn't use
    virtual unsigned long int getFrequencySwitching() { assert("Do not use it" == ""); return 0; /* suppress compiler warning */ }
    virtual double getCurrentPowerConsumption() { assert("Do not use it" == ""); return 0.0; /* suppress compiler warning */ }
    virtual void setMaxPowerConsumption(double e) { assert("Do not use it" == ""); }
    virtual double getMaxConsumption() { assert("Do not use it" == ""); return 0.0; /* suppress compiler warning */  }
    virtual void check() { assert("Do not use it" == ""); }

};


  
} // namespace RTSim

#endif
