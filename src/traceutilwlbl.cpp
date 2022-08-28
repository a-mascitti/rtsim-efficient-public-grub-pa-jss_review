#include <cpu.hpp>
#include <traceutilwlbl.hpp>

namespace RTSim
{
    using namespace std;

    TraceUtilWlSynch::TraceUtilWlSynch(Island_BL *island, const string &filename)
        : TraceAscii(filename)
    {
        _island = island;

        _formerUtils = vector<double>(_island->getProcessors().size());
        _formerWorkloads = vector<string>(_island->getProcessors().size());
        for (int i = 0; i < _island->getProcessors().size(); i++)
        {
            CPU_BL *cpu = _island->getProcessors().at(i);
            _formerUtils.at(i) = cpu->getUtilizationReadyRunning() + cpu->getUtilization_active();
            _formerWorkloads.at(i) = cpu->getWorkload();
        }

        _os << "time cpu_name util_rr util_active wl" << endl;
    }

    TraceUtilWlSynch::~TraceUtilWlSynch()
    {
        // _os << _ss.str() << endl;
    }

    void TraceUtilWlSynch::onUtilOrWlChanged(bool isForced /*=false*/)
    {
        vector<CPU_BL *> changed;
        for (int i = 0; i < _island->getProcessors().size(); i++)
        {
            CPU_BL *cpu = _island->getProcessors().at(i);
            double util = cpu->getUtilizationReadyRunning() + cpu->getUtilization_active();
            if (_formerUtils.at(i) - util < 0.0000001 || _formerWorkloads.at(i) != cpu->getWorkload())
                changed.push_back(cpu);
        }

        if (changed.size() > 0 || isForced) // just a double check to reduce disk accesses
        {
            for (int i = 0; i < _island->getProcessors().size(); i++)
            {
                CPU_BL *cpu = _island->getProcessors().at(i);

                _formerWorkloads.at(i) = cpu->getWorkload();
                _formerUtils.at(i) = cpu->getUtilizationReadyRunning() + cpu->getUtilization_active();
            }

            // kept separated to reduce disk accesses
            for (const auto &cpu : _island->getProcessors())
                _os << SIMUL.getTime()
                    << " " << cpu->getName()
                    << " " << cpu->getUtilizationReadyRunning()
                    << " " << cpu->getUtilization_active()
                    << " " << cpu->getWorkload()
                    << endl;
        }
    }
} // namespace RTSim
