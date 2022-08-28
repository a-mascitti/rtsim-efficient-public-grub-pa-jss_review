// Paper simulations, ex experiment 25

#include <config.hpp>
#include <cstring>
#include <string>
#include <fstream>

#include <mrtkernel.hpp>
#include <cpu.hpp>
#include <edfsched.hpp>
#include <jtrace.hpp>
#include <texttrace.hpp>
#include <json_trace.hpp>
#include <tracefreqbl.hpp>
#include <ps_trace.hpp>
#include <tracepowerbl.hpp>
#include <rttask.hpp>
#include <instr.hpp>
#include <powermodel.hpp>
#include <energyMRTKernel.hpp>
#include <assert.h>
#include "rrsched.hpp"
#include "cbserver.hpp"
#include <taskstat.hpp>
#include <fileImporter.hpp>
#include <unistd.h>
#include <ctime>

using namespace MetaSim;
using namespace RTSim;

#define REQUIRE assert
#define time() SIMUL.getTime()
#define MAX_HYPERPERIOD 5000000000 /* half billion simulation steps */

void dumpSpeeds(CPUModelBP::ComputationalModelBPParams const &params);

void dumpAllSpeeds();
bool isInRange(int, int);
bool isInRange(Tick t1, int t2) { return isInRange(int(t1), t2); }
bool isInRange(Tick t1, Tick t2) { return isInRange(int(t1), int(t2)); }
bool isInRangeMinMax(double eval, const double min, const double max);

void getCores(vector<CPU_BL *> &cpus_little, vector<CPU_BL *> &cpus_big, Island_BL **island_bl_little, Island_BL **island_bl_big);
vector<CPU_BL *> init_suite(EnergyMRTKernel **kern);

/// hyperperiod = lcm of periods, given as array
uint64_t hyperperiod(vector<unsigned long> periods);

#define dbg_system(args...)              \
	do                                   \
	{                                    \
		if (system(args) < 0)            \
		{                                \
			perror("system() failed: "); \
			exit(1);                     \
		}                                \
	} while (0)

extern int64_t BEGTICK;

int main(int argc, char *argv[])
{
	bool ONLY_LAST_ONE = true;		 // shall a new taskset be generated?
	unsigned int EXPERIMENTS_NO = 1; // # experiments to be performed (# paper simulations)
	int EXPERIMENT_TO_REDO = -1;	 // deletes the old result of the experiment, generates a new tasksset and performs the experiment
	double TOTAL_UTILIZATION = 0.2;	 // total utilization of generated tasksets for paper simulations
	unsigned int NTASK = 4;			 // # tasks per core, 4
	unsigned int NCPU = 6;			 // # cores, 6
	bool HARD_DEADLINES = false;	 // soft deadlines
	bool TEXTTRACE = true;			 // activates TextTrace for both kernels
	bool TRACE_UTIL_WL = false;		 // activates the util & wl tracer for the cpus
	bool PLOTSCHED = false;			 // is PlotSched tracing active (takes much computation)
	Tick PLOTSCHED_FROM = Tick(0);	 // tick when tracing begins. Tracing active by default from the beginning
        bool isMRTKernelFaster = 1; // make MRTKernel faster by not making checks at each tick

	// debug features
	int64_t STEP_DELTA = 0;					  // You can run exps given delta steps by delta steps
	int64_t UNTIL_TIME = 0;					  // executes the experiment until a given time, then stops and prints the state of the queues of the cores
	string EMRTKExecMode = "wholesimulation"; // How do you want to perform the simulation?

	string FOLDER_EMRTK[] 			= { "consumptions/energymrtk/", "consumptions/energymrtk_bf/", "consumptions/energymrtk_ff/" }; // base folder where results of simulation are stored for energyMRTK
	string FOLDER_MRTK = "consumptions/mrtk/";		  // base folder where results of simulation are stored for MRTK (Linux 5.3.11)
	string FOLDER_GRUB_PA = "consumptions/grub_pa/";  // base folder for results of MRTK GRUB-PA (~ to SCHED_DEADLINE)

	char cwd[1024] = "";
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		abort();
	cout << "This program has been tested on a machine with Linux Ubuntu 18.04.1 64 bit, gcc 7.4.0, make 4.1, Intel i7 and 16GB RAM. Simulations took some hours." << endl;
	cout << "Please just launch git clone https:://repo.com; cd rtsim/src/ ; ./launch_paper_sim.sh" << endl;
	cout << "Results will be saved in " << cwd << endl;
	cout << "Credits for code: A.Mascitti, T.Cucinotta, M.Marinoni 2019-2020" << endl;
	cout << endl;

	argc--;
	argv++;
	while (argc > 0)
	{
		if (strcmp(argv[0], "-n") == 0 || strcmp(argv[0], "--new-task-set") == 0)
		{ // paper simulations
			ONLY_LAST_ONE = false;
			cout << "Generating new taskset" << endl;
			dbg_system("rm -rf taskset_generator/*.txt");
			if (system("rm -rf consumptions/ log.txt trace25.txt taskgen_output") < 0)
			{
				cerr << "could not remove previous results. Abort" << endl;
				abort();
			}
		}
		else if (strcmp(argv[0], "-u") == 0 || strcmp(argv[0], "--until-time") == 0)
		{
			argc--;
			argv++;
			UNTIL_TIME = atoi(argv[0]);
			EMRTKExecMode = "untiltime";
		}
		else if (strcmp(argv[0], "-s") == 0 || strcmp(argv[0], "--start-from") == 0)
		{
			argc--;
			argv++;
			BEGTICK = atoi(argv[0]);
			cout << "Gonna print debugging info to file from tick " << BEGTICK << endl;
		}
		else if (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "help") == 0)
		{
			cout << "Runs the experiment. For example:" << endl;
			cout << "./a.out -n\t\tRuns the simulation for papers with a new taskset, which is randomly generated. If so, exp=25 always" << endl;
			cout << "./a.out -u 15000\tRuns the simulation for papers (exp.25) until the simulation step 15000 (i.e., for 15000 sim.steps)" << endl;
			cout << "./a.out -e 25\t\tRuns simulation for paper" << endl;
			cout << "./a.out -h\t\tRuns simulation for paper 100 by 100 steps, letting you see the status of the scheduler. Use also with -e and -u  ;)" << endl;
			return 0;
		}
		else if (strcmp(argv[0], "--total-utilization") == 0)
		{ // paper simulations
			argc--;
			argv++;
			TOTAL_UTILIZATION = atof(argv[0]);
			cout << "Total utilization " << TOTAL_UTILIZATION << endl;
			if (EXPERIMENT_TO_REDO != -1)
			{
				// args order is important for building up FOLDER_*MRTK
				cerr << "--total-utilization must be passed before --do-experiment" << endl;
				abort();
			}
		}
		else if (strcmp(argv[0], "--ncpu") == 0)
		{
			argc--;
			argv++;
			NCPU = atoi(argv[0]);
			cout << "nCPUs=" << NCPU << endl;
		}
		else if (strcmp(argv[0], "--ntask") == 0)
		{
			argc--;
			argv++;
			NTASK = atoi(argv[0]);
			cout << "nTasks=" << NTASK << endl;
		}
		else if (strcmp(argv[0], "--plotsched-from") == 0)
		{
			argc--;
			argv++;
			PLOTSCHED = true;
			PLOTSCHED_FROM = Tick(atoi(argv[0]));
			cout << "PlotSched from " << PLOTSCHED_FROM << endl;
		}
		else if (strcmp(argv[0], "--texttrace") == 0)
		{
			TEXTTRACE = true;
		}
		else if (strcmp(argv[0], "--trace-util") == 0)
		{
			TRACE_UTIL_WL = true;
		}
		else
		{
			cerr << "Unrecognized cmd option" << endl;
			abort();
		}
		argc--;
		argv++;
	}

	// configuration for simulation
	bool RAND_PER = true;				// false if you want all tasks of the generated taskset to have same period
	unsigned int PERIOD_GRAN = 500000u; // period granularity of the generated taskset, in ns
	EDFScheduler::threshold = 1.0;		//0.95; // ceilings...
	bool EMRTKERNEL = true;				// perform same experiment with EMRTK-kernel?
	bool MRTKERNEL = true;				// perform same experiment with G-EDF-FF?
	bool GRUB_PA = true;				// perform same experiment with GRUB_PA?
	vector<unsigned long> periods;		// tasks periods

	// configuration of the kernel
	EnergyMRTKernel::EMRTK_BALANCE_ENABLED = 0;
	EnergyMRTKernel::EMRTK_LEAVE_LITTLE3_ENABLED = 0;
	EnergyMRTKernel::EMRTK_MIGRATE_ENABLED = 1; // 1
	EnergyMRTKernel::EMRTK_CBS_YIELD_ENABLED = 0;
	EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_VTIME = 0;
	EnergyMRTKernel::EMRTK_TEMPORARILY_MIGRATE_END = 0;

	EnergyMRTKernel::EMRTK_CBS_ENVELOPING_MIGRATE_AFTER_VTIME_END = 1;	// 1
	EnergyMRTKernel::EMRTK_CBS_MIGRATE_AFTER_END = 1;					// 1
	EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_EMPTIEST_BIG = 0;	// 0
	EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG = 1;	// 1
	EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_LITTLE = 1; // 1
	EnergyMRTKernel::EMRTK_CBS_MIGRATE_BIGGEST_FROM_FULLEST_BIG_ONLY_FIRST = 1;
	EnergyMRTKernel::EMRTK_CBS_ENVELOPING_PER_TASK_ENABLED = 1;

	EnergyMRTKernel::EMRTK_TRY_KEEP_OLD_CPU = 1;
	EnergyMRTKernel::EMRTK_ENLARGE_WCET = 1;
	EnergyMRTKernel::EMRTK_ENLARGE_WCET_10_PERC = 0; // 0
	EnergyMRTKernel::EMRTK_ENLARGE_WCET_UNIF = 1;	 // 1

	EnergyMRTKernel::EMRTK_ABORT_ON_NOT_SCHEDULABLE = 0; // 0, simply skip current period

	EnergyMRTKernel::EMRTK_FORGET_UACTIVE_ON_CORE_FREE = 1;
	EnergyMRTKernel::EMRTK_FREQUENCY_SCALE_DOWN = 1;

	EnergyMRTKernel::EMRTK_PLACEMENT_LITTLE_FIRST = 0; // 0

	EnergyMRTKernel::EMRTK_PRINT_PLACEMENT_TIME = 1; // 1
	EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_WF 								= 1; // 1
	EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_BF 								= 0; // 0
	EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_FF 								= 0; // 0

	// variables shared by EMRTKernel and MRTKernel
	vector<Scheduler *> schedulers;
	vector<RTKernel *> kernels;
	vector<CPU_BL *> cpus_little, cpus_big;
	vector<AbsRTTask *> tasks;
	PeriodicTask *t;
	vector<CBServerCallingEMRTKernel *> ets;
	string task_name;
	EnergyMRTKernel *kern;

	// run the experiments with both kernels (energy- and un-aware)
	uint64_t simulation_steps = 0;
	string filename = "";
	int i = 0;

	cout << "Performing simulation " << EXPERIMENT_TO_REDO << " for paper simulation" << endl;

	if (!ONLY_LAST_ONE)
	{
		bool bad = true;
		do
		{
			cout << "Generating random taskset.." << endl;
			filename = StaffordImporter::generate(500, TOTAL_UTILIZATION, PERIOD_GRAN, MAX_HYPERPERIOD, RAND_PER, NCPU, NTASK); // period, utilization
			bad = !StaffordImporter::check(filename);
			if (bad)
				cout << "\tTaskset is bad. doing it again.." << endl;
			else
				cout << "\tCheck ok" << endl;
		} while (bad);
	}

	if (EMRTKERNEL)
	{
		vector<string> edfTypes = { "", "_bf", "_ff" };
		for (int it = 0; it < edfTypes.size(); it++) 
		{
			cerr << "EnergyMRTKernel " << edfTypes.at(it) << endl;

			dbg_system(("rm -rf " + FOLDER_EMRTK[it] + "*").c_str());
			if (system(("mkdir -p " + FOLDER_EMRTK[it] + to_string(0)).c_str()) < 0)
			{
				cerr << "Error creating results folder" << endl;
				abort();
			}

			TextTrace emrtkttrace(FOLDER_EMRTK[it] + to_string(i) + "/trace25_emrtk" + edfTypes.at(it) + ".txt");
			PSTrace *pstrace = new PSTrace("trace_emrtk_" + to_string(i) + edfTypes[it] + ".pst");
			pstrace->setStartingTick(PLOTSCHED_FROM);

			EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_WF = 0; // 1
			EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_BF = 0; // 0
			EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_FF = 0; // 0
			if (it == 1)
				EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_BF = 1;
			else if (it == 2)
				EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_FF = 1;
			else
				EnergyMRTKernel::EMRTK_PLACEMENT_P_EDF_WF = 1;

			vector<MissCount *> mc;
			EnergyMRTKernel *kern;
			init_suite(&kern);
			REQUIRE(kern != NULL);

			// load CPUs and islands
			vector<CPU_BL *> cpus_little = kern->getIslandLittle()->getProcessors();
			vector<CPU_BL *> cpus_big = kern->getIslandBig()->getProcessors();
			vector<CPU_BL *> cpus;
			cpus.reserve(cpus_little.size() + cpus_big.size());
			cpus.insert(cpus.end(), cpus_little.begin(), cpus_little.end());
			cpus.insert(cpus.end(), cpus_big.begin(), cpus_big.end());

			vector<Island_BL *> islands = {kern->getIslandBig(), kern->getIslandLittle()};

			for (const auto &isl : islands)
			{
				isl->setSynchPowerTracerDirectory(FOLDER_EMRTK[it] + to_string(i) + "/");
				isl->setSynchFreqTracerDirectory(FOLDER_EMRTK[it] + to_string(i) + "/");
				if (TRACE_UTIL_WL)
					isl->setSynchUtilWlTracerDirectory(FOLDER_EMRTK[it] + to_string(i) + "/");
			}

			// redirect stdout to study it
			cout << "Redirecting std output to " << FOLDER_EMRTK[it] << i << "/log_emrtk_1_run.txt" << endl;
			ofstream out(FOLDER_EMRTK[it] + to_string(i) + "/log_emrtk_1_run.txt", fstream::app);
			streambuf* olds = cout.rdbuf(out.rdbuf()); // attention: cout redirected to file to study and debug the output

			// the experiment begins here.
			cout << "Using former taskset..." << endl;
			filename = StaffordImporter::getLastGenerated();
			try
			{
				ets = StaffordImporter::getEnvelopedPeriodcTasks(filename, kern, i);
			}
			catch (std::exception &e)
			{
				cerr << "Error: " << e.what() << endl;
				abort();
			}
			assert(ets.size() > 0);
			cout << "Got " << ets.size() << " tasks" << endl;

			EnergyMRTKernel *k = dynamic_cast<EnergyMRTKernel *>(kern);

			// install trackers into tasks
			for (CBServerCallingEMRTKernel *t : ets)
			{
				AbsRTTask *firstTask = t->getAllTasks().at(0);
				periods.push_back((unsigned long)double(firstTask->getPeriod()));
				tasks.push_back(firstTask);
				if (TEXTTRACE)
					emrtkttrace.attachToTask(*firstTask);
				if (PLOTSCHED)
					t->setPSTrace(pstrace);

				MissCount *misscount = new MissCount("deadline misses counter for task " + taskname(firstTask));
				misscount->attachToTask(dynamic_cast<Task *>(firstTask));
				mc.push_back(misscount);
				dynamic_cast<Task *>(firstTask)->setMissCount(misscount);

				dynamic_cast<Task *>(firstTask)->setAbort(HARD_DEADLINES);
			}

			if (PLOTSCHED)
			{
				vector<CBServer *> cbses;
				for (const auto &t : ets)
					cbses.push_back(t);

				pstrace->writeTasks(cbses, tasks, "./tasks.txt");
				pstrace->writeCPUs(islands, "./cpus.txt");
			}

			// do all experiments deal with the same tasks
			kern->printAllTasks(FOLDER_EMRTK[it] + "/0/all_tasks.txt");

			// you can run the experiment in different modalities. Pick yours!
			clock_t beg = clock();
			SIMUL.initSingleRun();
			simulation_steps = hyperperiod(periods);
			cerr << "simulation_steps=" << simulation_steps << endl;
			if (simulation_steps > MAX_HYPERPERIOD)
			{ // 500 millions
				cerr << "Too many simulation steps: " << simulation_steps << endl;
				abort();
			}
			cout << endl
				 << "simulation_steps=" << simulation_steps << endl;
			cout << "using file " << filename << endl;
			if (EMRTKExecMode == "stepbystep")
			{
				for (uint64_t simstep = 0; simstep < simulation_steps; simstep++)
				{
					SIMUL.run_to(Tick::ceil(simstep));
					k->printState(1);
				}
			}
			else if (EMRTKExecMode == "wholesimulation")
			{
				SIMUL.run_to(Tick::ceil(simulation_steps));
			}
			else if (EMRTKExecMode == "stepbystepdelta")
			{
				for (uint64_t simstep = 0; simstep < simulation_steps; simstep += STEP_DELTA)
				{
					SIMUL.run_to(Tick::ceil(simstep));
					k->printState(1);
				}
			}
			else if (EMRTKExecMode == "untiltime")
			{
				cout << "Running until time " << UNTIL_TIME << endl;
				SIMUL.run_to(Tick(UNTIL_TIME));
				k->printState(1, 1);
			}

//			islands[0]->onFreqOrWlChanged(islands[0]->getProcessors().at(0));
//			islands[1]->onFreqOrWlChanged(islands[1]->getProcessors().at(0));
//			islands[0]->onUtilOrWlChanged();
//			islands[1]->onUtilOrWlChanged();
			islands[0]->storeCurFrequency();
			islands[1]->storeCurFrequency();
			
			SIMUL.endSingleRun();
			clock_t end = clock();

			// print experiment results
			double elapsed_secs = double(end - beg) / CLOCKS_PER_SEC;
			cout << "Simulation took seconds: " << elapsed_secs << endl;

			unsigned int frequencyIncreases[3] = {0, 0, 0};
			frequencyIncreases[0] = k->getFrequencyIncreasesBig();
			frequencyIncreases[1] = k->getFrequencyIncreasesLittle();
			frequencyIncreases[2] = frequencyIncreases[0] + frequencyIncreases[1];

			unsigned int migrationNumber = k->getEnergyMultiCoresScheds()->getMigrationNumber();
			unsigned int migrationPoints = k->getNumberEndEvents() + k->getNumberVtimeEvents(); // # vtime exp evt <= # end evt: some vtime may be in the past
			unsigned int missing = 0;
			for (MissCount *c : mc)
			{
				missing += c->getLastValue();
				delete c;
			}

			cout << "--------------" << endl;
			cout << "missing tasks: " << missing
				 << ", not schedulable tasks: " << k->getTasksNonSchedulable()
				 << ", migrations: " << migrationNumber
				 << ", total # of possibile migration point: " << migrationPoints
				 << ", frequency increases big: " << frequencyIncreases[0]
				 << ", frequency increases litte: " << frequencyIncreases[1]
				 << ", total frequency increases: " << frequencyIncreases[2]
				 << endl;
			cout << "Simulation finished (EMRTK), filename=" << filename << endl;
			if (HARD_DEADLINES)
				assert(missing == k->getTasksNonSchedulable()); // missing == 0 + # of non-schedulable tasks because of overload

			for (AbsRTTask *t : tasks)
				delete t;
			for (CBServerCallingEMRTKernel *cbs : ets)
				delete cbs;
			delete pstrace;
			delete kern; // destroys islands, cores
			periods.clear();
			vector<Island_BL *>().swap(islands);
			vector<CPU_BL *>().swap(cpus);
			vector<Scheduler *>().swap(schedulers);
			vector<AbsRTTask *>().swap(tasks);
			vector<CBServerCallingEMRTKernel *>().swap(ets);
			cout.rdbuf(olds);
			//return 0;
		}
	}

	// now we try example with MRTKernel
	if (MRTKERNEL)
	{
		cerr << "MRTKernel" << endl;

		dbg_system(("rm -rf " + FOLDER_MRTK + "*").c_str());
		if (system(("mkdir -p " + FOLDER_MRTK + to_string(0)).c_str()) < 0)
		{
			cerr << "Error creating results folder" << endl;
			abort();
		}

		ofstream out(FOLDER_MRTK + to_string(i) + "/log_mrtk_1_run.txt", fstream::app);
		cout.rdbuf(out.rdbuf());

		TextTrace mrtkttrace(FOLDER_MRTK + to_string(i) + "/trace25_mrtk.txt");
		vector<MissCount *> mc;
		vector<CPU_BL *> temp = init_suite(NULL);
		vector<CPU *> cpus;
		vector<Island_BL *> islands = {dynamic_cast<CPU_BL *>(temp.at(0))->getIsland(), dynamic_cast<CPU_BL *>(temp.at(6))->getIsland()};

		for (const auto &isl : islands)
		{
			isl->setSynchPowerTracerDirectory(FOLDER_MRTK + to_string(i) + "/");
			isl->setSynchFreqTracerDirectory(FOLDER_MRTK + to_string(i) + "/");
			if (TRACE_UTIL_WL)
				isl->setSynchUtilWlTracerDirectory(FOLDER_MRTK + to_string(i) + "/");
		}

		for (CPU_BL *c : temp)
		{
			c->setMaxFrequency();
			cpus.push_back(static_cast<CPU *>(c));
		}
		assert(cpus.size() == 8);

		EDFScheduler *edfsched = new EDFScheduler;
		schedulers.push_back(edfsched);

		MRTKernel_Linux5_3_11 *kern = new MRTKernel_Linux5_3_11(edfsched, cpus, "");
		kernels.push_back(kern);

		string filename = StaffordImporter::getLastGenerated();

		vector<PeriodicTask *> per_tasks;
		try
		{
			per_tasks = StaffordImporter::getPeriodicTasks(filename, i);
		}
		catch (std::exception &e)
		{
			cerr << "Error: " << e.what() << endl;
			return 0;
		}

		for (AbsRTTask *t : per_tasks)
		{
			tasks.push_back(t);
			kern->addTask(*t, ""); // not a CBS

			Task *tt = dynamic_cast<Task *>(t);
			if (TEXTTRACE)
				mrtkttrace.attachToTask(*t);

			MissCount *misscount = new MissCount("deadline misses counter for task " + taskname(t));
			misscount->attachToTask(tt);
			mc.push_back(misscount);
		}
		cout << "MRTKernel_Linux5_3_11. Got " << tasks.size() << " tasks." << endl;
		cout << "Simulation steps: " << simulation_steps << endl;
		assert(tasks.size() > 0);

		clock_t beg = clock();
		SIMUL.initSingleRun();
		if (isMRTKernelFaster)
			SIMUL.run_to(Tick::ceil(simulation_steps));
		else
			for (int32_t y = 0; y <= simulation_steps; y++)
			{
				SIMUL.run_to(y);
				for (CPU_BL *c : temp)
					assert(c->getFrequency() == 2000 || c->getFrequency() == 1400);
			}
		SIMUL.endSingleRun();
		clock_t end = clock();
		double elapsedSec = (end - beg) / CLOCKS_PER_SEC;
		cout << "Simulation steps: " << simulation_steps << endl;
		cout << "Simulation took seconds: " << elapsedSec << endl;

		unsigned int missing = 0;
		for (MissCount *c : mc)
		{
			missing += c->getLastValue();
			delete c;
		}
		vector<MissCount *>().swap(mc);

		assert(missing == 0);
		cout << "--------------" << endl;
		cout << "missing tasks: " << missing << endl;
		cout << "i = " << i << endl;
		cout << "Simulation finished (MRTK), filename=" << filename << endl;

		for (AbsRTTask *t : tasks)
			delete t;
		vector<AbsRTTask *>().swap(tasks);
		for (Island_BL *b : islands)
			delete b;
		vector<Island_BL *>().swap(islands);
		delete kern, edfsched;
		vector<CPU *>().swap(cpus);
		vector<Scheduler *>().swap(schedulers);
	}

	// now we try example with GRUB-PA
	if (GRUB_PA)
	{
		cerr << "grub-pa" << endl;

		dbg_system(("rm -rf " + FOLDER_GRUB_PA + "*").c_str());
		if (system(("mkdir -p " + FOLDER_GRUB_PA + to_string(0)).c_str()) < 0)
		{
			cerr << "Error creating results folder" << endl;
			abort();
		}

		// Redirecting stdout to file
		ofstream out(FOLDER_GRUB_PA + to_string(i) + "/log_mrtk_1_run.txt", fstream::app);
		cout.rdbuf(out.rdbuf());

		TextTrace mrtkttrace(FOLDER_GRUB_PA + to_string(i) + "/trace25_mrtk.txt");
		PSTrace *pstrace = new PSTrace("trace_emrtk_grubpa_" + to_string(i) + ".pst");
		pstrace->setStartingTick(PLOTSCHED_FROM);
		vector<MissCount *> mc;
		vector<CPU_BL *> temp = init_suite(NULL);
		vector<CPU *> cpus;
		vector<Island_BL *> islands = {dynamic_cast<CPU_BL *>(temp.at(0))->getIsland(), dynamic_cast<CPU_BL *>(temp.at(6))->getIsland()};

		for (const auto &isl : islands)
		{
			isl->setSynchPowerTracerDirectory(FOLDER_GRUB_PA + to_string(i) + "/");
			isl->setSynchFreqTracerDirectory(FOLDER_GRUB_PA + to_string(i) + "/");
			if (TRACE_UTIL_WL)
				isl->setSynchUtilWlTracerDirectory(FOLDER_GRUB_PA + to_string(i) + "/");
		}

		for (CPU_BL *c : temp)
		{
			c->setMaxFrequency();
			cpus.push_back(static_cast<CPU *>(c));
		}
		assert(cpus.size() == 8);

		for (int i = 0; i < 8; i++)
			schedulers.push_back(new EDFScheduler());

		EDFScheduler *edfsched = new EDFScheduler;

		MRTKernel_Linux5_3_11_GRUB_PA *kern = new MRTKernel_Linux5_3_11_GRUB_PA(schedulers, edfsched, islands.at(1), islands.at(0));
		kernels.push_back(kern);

		string filename = StaffordImporter::getLastGenerated();

		try
		{
			ets = StaffordImporter::getEnvelopedPeriodcTasks(filename, kern, i);
		}
		catch (
			std::exception &e)
		{
			cerr << "Error: " << e.what() << endl;
			abort();
		}
		assert(ets.size() > 0);
		cout << "Got " << ets.size() << " tasks" << endl;

		// install trackers into tasks
		for (CBServerCallingEMRTKernel *t : ets)
		{
			Task *firstTask = dynamic_cast<Task *>(t->getAllTasks().at(0));
			tasks.push_back(firstTask);
			periods.push_back((unsigned long)double(firstTask->getPeriod()));
			// kern->addTask(*firstTask, ""); // not a CBS

			if (TEXTTRACE)
				mrtkttrace.attachToTask(*firstTask);
			if (PLOTSCHED)
				t->setPSTrace(pstrace);

			MissCount *misscount = new MissCount("deadline misses counter for task " + taskname(t));
			misscount->attachToTask(firstTask);
			mc.push_back(misscount);

			firstTask->setMissCount(misscount);
			firstTask->setAbort(HARD_DEADLINES);
		}
		simulation_steps = hyperperiod(periods);
		assert(tasks.size() > 0);

		if (PLOTSCHED)
		{
			vector<CBServer *> cbses;
			for (const auto &t : ets)
				cbses.push_back(t);

			pstrace->writeTasks(cbses, tasks, "./tasks.txt");
			pstrace->writeCPUs(islands, "./cpus.txt");
		}

		// see if all simulations deal with the same tasks
		kern->printAllTasks(FOLDER_GRUB_PA + "/0/all_tasks.txt");

		clock_t beg = clock();
		SIMUL.initSingleRun();
		if (EMRTKExecMode == "wholesimulation")
			SIMUL.run_to(Tick(double(simulation_steps)));
		else if (EMRTKExecMode == "untiltime")
		{
			cout << "Running until time " << UNTIL_TIME << endl;
			SIMUL.run_to(Tick(UNTIL_TIME));
			kern->printState(1, 1);
			cout << Event::strQueue() << endl;
		}

//		islands[0]->onFreqOrWlChanged(islands[0]->getProcessors().at(0));
//		islands[1]->onFreqOrWlChanged(islands[1]->getProcessors().at(0));
//		islands[0]->onUtilOrWlChanged();
//		islands[1]->onUtilOrWlChanged();
		islands[0]->storeCurFrequency();
		islands[1]->storeCurFrequency();

		SIMUL.endSingleRun();
		clock_t end = clock();
		double elapsedSec = (end - beg) / CLOCKS_PER_SEC;
		cout << "Simulation steps: " << simulation_steps << endl;
		cout << "Simulation took seconds: " << elapsedSec << endl;

		unsigned int frequencyIncreases[3] = {0, 0, 0};
		frequencyIncreases[0] = kern->getFrequencyIncreasesBig();
		frequencyIncreases[1] = kern->getFrequencyIncreasesLittle();
		frequencyIncreases[2] = frequencyIncreases[0] + frequencyIncreases[1];

		unsigned int migrationNumber = kern->getEnergyMultiCoresScheds()->getMigrationNumber();
		unsigned int migrationPoints = kern->getNumberEndEvents() + kern->getNumberVtimeEvents(); // # vtime exp evt <= # end evt: some vtime may be in the past
		unsigned int missing = 0;
		for (MissCount *c : mc)
		{
			missing += c->getLastValue();
			delete c;
		}

		cout << "--------------" << endl;
		cout << "missing tasks: " << missing << ", not schedulable tasks: " << kern->getTasksNonSchedulable() << ", migrations: " << migrationNumber << ", total # of possibile migration point: " << migrationPoints << ", frequency increases big: " << frequencyIncreases[0] << ", frequency increases litte: " << frequencyIncreases[1] << ", total frequency increases: " << frequencyIncreases[2] << endl;
		cout << "i = " << i << endl;
		cout << "Simulation finished (" << kern->getClassName() << "), filename=" << filename << endl;
		if (HARD_DEADLINES)
			assert(missing == kern->getTasksNonSchedulable()); // missing == 0 + # of non-schedulable tasks because of overload

		for (AbsRTTask *t : tasks)
			delete t;
		for (CBServerCallingEMRTKernel *cbs : ets)
			delete cbs;
		delete kern;
		tasks.clear();
		ets.clear();
		periods.clear();
	}

	return 0;
}

void dumpSpeeds(CPUModelBP::ComputationalModelBPParams const &params)
{
	for (unsigned int f = 200000; f <= 2000000; f += 100000)
	{
		std::cout << "Slowness of " << f << " is " << CPUModelBP::slownessModel(params, f) << std::endl;
	}
}

void dumpAllSpeeds()
{
	std::cout << "LITTLE:" << std::endl;
	CPUModelBP::ComputationalModelBPParams bzip2_cp = {0.0256054, 2.9809e+6, 0.602631, 8.13712e+9};
	dumpSpeeds(bzip2_cp);
	std::cout << "BIG:" << std::endl;
	bzip2_cp = {0.17833, 1.63265e+6, 1.62033, 118803};
	dumpSpeeds(bzip2_cp);
}

/// Returns true if the value 'eval' and 'expected' are distant 'error'%
bool isInRange(int eval, int expected)
{
	const unsigned int error = 5;

	int min = int(eval - eval * error / 100);
	int max = int(eval + eval * error / 100);

	return expected >= min && expected <= max;
}

/// True if min <= eval <= max
bool isInRangeMinMax(double eval, const double min, const double max)
{
	return min <= eval <= max;
}

void getCores(vector<CPU_BL *> &cpus_little, vector<CPU_BL *> &cpus_big, Island_BL **island_bl_little, Island_BL **island_bl_big)
{
	unsigned int OPP_little = 0; // Index of OPP in LITTLE cores
	unsigned int OPP_big = 0;	 // Index of OPP in big cores

	// frequencies in Hz and voltage in volts I guess, if then power is in Watt
	vector<double> V_little = {
		0.92, 0.919643, 0.919357, 0.918924, 0.95625, 0.9925, 1.02993, 1.0475, 1.08445, 1.12125, 1.15779, 1.2075,
		1.25625};
	vector<unsigned int> F_little = {
		200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400};

	vector<double> V_big = {
		0.916319, 0.915475, 0.915102, 0.91498, 0.91502, 0.90375, 0.916562, 0.942543, 0.96877, 0.994941, 1.02094,
		1.04648, 1.05995, 1.08583, 1.12384, 1.16325, 1.20235, 1.2538, 1.33287};
	vector<unsigned int> F_big = {
		200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000};

	if (OPP_little >= V_little.size() || OPP_big >= V_big.size())
		exit(-1);

	unsigned long int max_frequency = max(F_little[F_little.size() - 1], F_big[F_big.size() - 1]);

	/* ------------------------- Creating CPU_BLs -------------------------*/
	for (unsigned int i = 0; i < 4; ++i)
	{
		/* Create 4 LITTLE CPU_BLs */
		string cpu_name = "L" + to_string(i);

		// cout << "Creating CPU_BL: " << cpu_name << endl;
		// cout << "f is " << F_little[F_little.size() - 1] << " max_freq " << max_frequency << endl;

		CPUModelBP *pm = new CPUModelBP(V_little[V_little.size() - 1], F_little[F_little.size() - 1], max_frequency);
		{
			CPUModelBP::PowerModelBPParams idle_pp = {0.00134845, 1.76307e-5, 124.535, 1.00399e-10};
			CPUModelBP::ComputationalModelBPParams idle_cp = {1, 0, 0, 0};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("idle", idle_pp, idle_cp);

			CPUModelBP::PowerModelBPParams bzip2_pp = {0.00775587, 33.376, 1.54585, 9.53439e-10};
			CPUModelBP::ComputationalModelBPParams bzip2_cp = {0.0256054, 2.9809e+6, 0.602631, 8.13712e+9};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp, bzip2_cp);

			CPUModelBP::PowerModelBPParams hash_pp = {0.00624673, 176.315, 1.72836, 1.77362e-10};
			CPUModelBP::ComputationalModelBPParams hash_cp = {0.00645628, 3.37134e+6, 7.83177, 93459};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("hash", hash_pp, hash_cp);

			CPUModelBP::PowerModelBPParams encrypt_pp = {0.00676544, 26.2243, 5.6071, 5.34216e-10};
			CPUModelBP::ComputationalModelBPParams encrypt_cp = {6.11496e-78, 3.32246e+6, 6.5652, 115759};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp, encrypt_cp);

			CPUModelBP::PowerModelBPParams decrypt_pp = {0.00629664, 87.1519, 2.93286, 2.80871e-10};
			CPUModelBP::ComputationalModelBPParams decrypt_cp = {5.0154e-68, 3.31791e+6, 7.154, 112163};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp, decrypt_cp);

			CPUModelBP::PowerModelBPParams cachekiller_pp = {0.0126737, 67.9915, 1.63949, 3.66185e-10};
			CPUModelBP::ComputationalModelBPParams cachekiller_cp = {1.20262, 352597, 2.03511, 169523};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp, cachekiller_cp);
		}

		// cout << "creating cpu" << endl;
		CPU_BL *c = new CPU_BL(cpu_name, "idle", pm);
		c->setIndex(i);
		pm->setCPU(c);
		pm->setFrequencyMax(max_frequency);

		cpus_little.push_back(c);
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		/* Create 4 big CPU_BLs */

		string cpu_name = "B" + to_string(i);

		// cout << "Creating CPU_BL: " << cpu_name << endl;

		CPUModelBP *pm = new CPUModelBP(V_big[V_big.size() - 1], F_big[F_big.size() - 1], max_frequency);
		{
			CPUModelBP::PowerModelBPParams idle_pp = {0.0162881, 0.00100737, 55.8491, 1.00494e-9};
			CPUModelBP::ComputationalModelBPParams idle_cp = {1, 0, 0, 0};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("idle", idle_pp, idle_cp);

			CPUModelBP::PowerModelBPParams bzip2_pp = {0.0407739, 12.022, 3.33367, 7.4577e-9};
			CPUModelBP::ComputationalModelBPParams bzip2_cp = {0.17833, 1.63265e+6, 1.62033, 118803};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("bzip2", bzip2_pp, bzip2_cp);

			CPUModelBP::PowerModelBPParams hash_pp = {0.0388215, 16.3205, 4.3418, 5.07039e-9};
			CPUModelBP::ComputationalModelBPParams hash_cp = {0.017478, 1.93925e+6, 4.22469, 83048.3};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("hash", hash_pp, hash_cp);

			CPUModelBP::PowerModelBPParams encrypt_pp = {0.0348728, 8.14399, 5.64344, 7.69915e-9};
			CPUModelBP::ComputationalModelBPParams encrypt_cp = {8.39417e-34, 1.99222e+6, 3.33002, 96949.4};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("encrypt", encrypt_pp, encrypt_cp);

			CPUModelBP::PowerModelBPParams decrypt_pp = {0.0320508, 25.8727, 3.27135, 4.11773e-9};
			CPUModelBP::ComputationalModelBPParams decrypt_cp = {9.49471e-35, 1.98761e+6, 2.65652, 109497};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("decrypt", decrypt_pp, decrypt_cp);

			CPUModelBP::PowerModelBPParams cachekiller_pp = {0.086908, 9.17989, 2.5828, 7.64943e-9};
			CPUModelBP::ComputationalModelBPParams cachekiller_cp = {0.825212, 235044, 786.368, 25622.1};
			dynamic_cast<CPUModelBP *>(pm)->setWorkloadParams("cachekiller", cachekiller_pp, cachekiller_cp);
		}

		CPU_BL *c = new CPU_BL(cpu_name, "idle", pm);
		c->setIndex(i + 4);
		pm->setCPU(c);
		pm->setFrequencyMax(max_frequency);

		cpus_big.push_back(c);
	}

	vector<struct OPP> opps_little = Island_BL::buildOPPs(V_little, F_little);
	vector<struct OPP> opps_big = Island_BL::buildOPPs(V_big, F_big);
	*island_bl_little = new Island_BL("little island", IslandType::LITTLE, cpus_little, opps_little);
	*island_bl_big = new Island_BL("big island", IslandType::BIG, cpus_big, opps_big);
}

vector<CPU_BL *> init_suite(EnergyMRTKernel **kern)
{
	cout << "init_suite" << endl;

#if LEAVE_LITTLE3_ENABLED
	cout << "Error: tests thought for LEAVE_LITTLE3_ENABLED disabled" << endl;
	abort();
#endif

	Island_BL *island_bl_big = NULL, *island_bl_little = NULL;
	vector<CPU_BL *> cpus_little, cpus_big;
	vector<Scheduler *> schedulers;
	vector<RTKernel *> kernels;

	getCores(cpus_little, cpus_big, &island_bl_little, &island_bl_big);
	REQUIRE(island_bl_big != NULL);
	REQUIRE(island_bl_little != NULL);
	REQUIRE(cpus_big.size() == 4);
	REQUIRE(cpus_little.size() == 4);

	EDFScheduler *edfsched = new EDFScheduler;
	for (int i = 0; i < 8; i++)
		schedulers.push_back(new EDFScheduler());

	if (kern != NULL)
	{
		*kern = new EnergyMRTKernel(schedulers, edfsched, island_bl_big, island_bl_little, "The sole kernel");
		kernels.push_back(*kern);
	}

	CPU_BL::REFERENCE_FREQUENCY = 2000;			  // BIG_3 frequency
	CPU_BL::REFERENCE_SPEED = 1.0053736425473412; // BIG 3 frequency 2000

	vector<CPU_BL *> cpus;
	cpus.insert(cpus.end(), cpus_little.begin(), cpus_little.end());
	cpus.insert(cpus.end(), cpus_big.begin(), cpus_big.end());

	cout << "end init_suite" << endl;
	return cpus;
}

uint32_t gcd(unsigned long a, unsigned long b)
{
	if (b == 0)
		return a;
	return gcd(b, a % b);
}

uint64_t hyperperiod(vector<unsigned long> periods)
{
	uint64_t ans;

	stringstream ss;
	ss << "python3 ../mcm.py \"";
	for (const auto &p : periods)
		ss << p << " ";
	ss << "\" > mcm_py_output.dat";
	dbg_system(ss.str().c_str());
	//cerr << "Executing " << endl << ss.str() << endl;

	ifstream f;
	string aa;
	f.open("mcm_py_output.dat");
	f >> aa;
	// cerr << "got " << aa << endl;
	ans = stoull(aa);
	f.close();
	// cerr << "got ans=" << ans << endl;

	for (const auto &p : periods)
		assert(ans % p == 0);

	return ans;
}
