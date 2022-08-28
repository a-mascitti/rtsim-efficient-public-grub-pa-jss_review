#include <cpu.hpp>
#include <tracepowerbl.hpp>

namespace RTSim {
    using namespace std;

    TracePowerConsumptionSynch::TracePowerConsumptionSynch(CPU_BL* cpu, const string &filename) 
        : TraceAscii(filename)
    {
        _cpu = cpu;
        _formerFreqs.push_back(cpu->getFrequency());
        _formerWorkloads.push_back(cpu->getWorkload());
    }

    TracePowerConsumptionSynch::TracePowerConsumptionSynch(Island_BL* island, const string &filename)
        : TraceAscii(filename)
    {
        _island = island;

        _formerFreqs = vector<unsigned int>(_island->getProcessors().size());
        _formerWorkloads = vector<string>(_island->getProcessors().size());
        for (int i = 0; i < _island->getProcessors().size(); i++)
        {
            _formerFreqs.at(i) = _island->getProcessors().at(i)->getFrequency();
            _formerWorkloads.at(i) = "idle";
        }
    }

    TracePowerConsumptionSynch::~TracePowerConsumptionSynch()
    {
        // _os << _ss.str() << endl;
    }

    void TracePowerConsumptionSynch::onFreqOrWlChanged()
    {
        // two tracing are possible, by core and by island

        if (_island != NULL)
        {
            bool doit = false;
            for (int i = 0; i < _island->getProcessors().size() && !doit; i++)
            {
                CPU_BL* cpu = _island->getProcessors().at(i);
                if (_formerFreqs.at(i) != cpu->getFrequency() || _formerWorkloads.at(i) != cpu->getWorkload())
                    doit = true;
            }

            if (doit) // just a double check to reduce disk accesses
            {
                double currentIslandConsumption = 0.0;
                for (int i = 0; i < _island->getProcessors().size(); i++)
                {
                    CPU_BL* cpu = _island->getProcessors().at(i);

                    _formerWorkloads.at(i) = cpu->getWorkload();
                    _formerFreqs.at(i)     = cpu->getFrequency();
                }

		// kept separated to reduce disk accesses
		for (const auto& cpu : _island->getProcessors())
                    currentIslandConsumption += cpu->getCurrentPowerConsumption(); // watt

                _os << currentIslandConsumption << " " << SIMUL.getTime() << endl; // watt tick:ns
            }
        }
        else
        {
            const string curWorkload = _cpu->getWorkload();
            const unsigned int curFrequency = _cpu->getFrequency();
            
            if (curWorkload != _formerWorkloads.at(0) || curFrequency != _formerFreqs.at(0)) { // just a double check to reduce disk accesses
                _formerWorkloads.at(0) = curWorkload;
                _formerFreqs.at(0)     = curFrequency;

                double currentPowerConsumption = _cpu->getCurrentPowerConsumption();
                _os << "Cur W=" << currentPowerConsumption << " t=" << SIMUL.getTime() << endl;
           }
       }
    }
}
