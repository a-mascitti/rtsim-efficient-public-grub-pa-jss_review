#include <texttrace.hpp>

namespace RTSim {
        using namespace std;
        using namespace MetaSim;

	    TextTrace::TextTrace(const string& name)
		{
			fd.open(name.c_str());
		}

		TextTrace::~TextTrace()
		{
			fd.close();
		}

		void TextTrace::probe(ArrEvt& e)
		{
			if (SIMUL.getTime() < startingTick) return;

			Task* tt = e.getTask();
            fd << "[Time:" << SIMUL.getTime() << "]\t";  
            fd << tt->getName() << " arrived at " 
               << tt->getArrival() << endl;                
		}

		void TextTrace::probe(EndEvt& e)
		{
			if (SIMUL.getTime() < startingTick) return;

			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";
			fd << tt->getName()<<" ended on " 
			   << tt->getKernel()->getOldProcessor(tt)->toString() 
			   << ", its arrival was " << tt->getArrival() 
			   << ", its period was " << tt->getPeriod() << endl;
		}

		void TextTrace::probe(SchedEvt& e)
		{
			if (SIMUL.getTime() < startingTick) return;

			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			CPU* c = tt->getKernel()->getProcessor(tt);
			if (c!= NULL) {
		        string frequency_str = "";
		      
		        if (dynamic_cast<CPU_BL*>(c))
		          	frequency_str = "freq " + to_string(dynamic_cast<CPU_BL*>(c)->getFrequency());

				fd << tt->getName()<<" scheduled on CPU " << c->getName() << " " << c->getSpeed() << frequency_str << " abs WCET "
					<< tt->getWCET() << " its arrival was " << tt->getArrival() << endl;
      		}
		}
  
		void TextTrace::probe(DeschedEvt& e)
		{
			if (SIMUL.getTime() < startingTick) return;

			Task* tt = e.getTask();
			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			fd << tt->getName()<<" descheduled its arrival was " 
				<< tt->getArrival() << endl;
		}

		void TextTrace::probe(DeadEvt& e)
		{
			if (SIMUL.getTime() < startingTick) return;
			
			Task* tt = e.getTask();

			fd << "[Time:" << SIMUL.getTime() << "]\t";  
			fd << tt->getName()<<" missed its arrival was " 
			   << tt->getArrival() << endl;
		}

		// you can call this manually from the extern
		void TextTrace::probe(string& s, Tick time)
		{
			fd << "[Time:" << time << "]\t" << s << endl;
		}

		void TextTrace::attachToTask(AbsRTTask &t)
		{
		    Task &tt = dynamic_cast<Task&>(t);
            attach_stat(*this, tt.arrEvt);
            attach_stat(*this, tt.endEvt);
            attach_stat(*this, tt.schedEvt);
            attach_stat(*this, tt.deschedEvt);
            attach_stat(*this, tt.deadEvt);
		}

		void TextTrace::setStartingTick(Tick &tick)
		{
			startingTick = tick;
		}
    
        VirtualTrace::VirtualTrace(map<string, int> *r)
        {
            results = r;
        }
        
        VirtualTrace::~VirtualTrace()
        {
            
        }
    
        void VirtualTrace::probe(EndEvt& e)
        {
            Task* tt = e.getTask();
            auto tmp_wcrt = SIMUL.getTime() - tt->getArrival();
            
            if ((*results)[tt->getName()] < tmp_wcrt)
            {
                (*results)[tt->getName()] = tmp_wcrt;
            }
        }

        void VirtualTrace::attachToTask(Task& t)
        {
            //new Particle<EndEvt, VirtualTrace>(&t->endEvt, this);
            attach_stat(*this, t.endEvt);
        }
    
};
