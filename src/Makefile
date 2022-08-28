CXXFLAGS_DEBUG=-Wno-deprecated -Wno-deprecated-declarations --std=c++11 -I. -g -O0 -DDEBUG
CXXFLAGS=-Wno-deprecated -Wno-deprecated-declarations --std=c++11 -I. -O3 -g

CPP_FILES=$(wildcard *.cpp)
HPP_FILES=$(wildcard *.hpp)
OBJ_FILES=$(patsubst %.cpp,%.o,$(CPP_FILES))

SOURCES=$(CPP_FILES) $(HPP_FILES)

PROGRAMS=paper_sim

.PHONY: all clean dep

all:  $(PROGRAMS) debug $(patsubst %,%_debug,$(PROGRAMS))

example: $(filter-out paper_sim.o,$(OBJ_FILES))
	g++ -o $@ $^

paper_sim: $(filter-out energyMRTKernel_example.o,$(OBJ_FILES))
	g++ -o $@ $^

debug:
	mkdir -p debug

OBJ_FILES_DEBUG=$(patsubst %.o,debug/%.o,$(OBJ_FILES))

debug/%.o: %.cpp
	g++ -c $(CXXFLAGS_DEBUG) -o $@ $<

paper_sim_debug: $(filter-out debug/energyMRTKernel_example.o,$(OBJ_FILES_DEBUG))
	g++ -o $@ $^

example_debug: $(filter-out debug/paper_sim.o,$(OBJ_FILES_DEBUG))
	g++ -o $@ $^

clean:
	rm -f example paper_sim paper_sim_debug *.o capacities.txt debug.txt *~ *.bak debug/*.o

dep:
	makedepend -- -I. -Y -- *.cpp

# DO NOT DELETE

AVRTask.o: ./AVRTask.hpp ./task.hpp ./entity.hpp ./baseexc.hpp ./basetype.hpp
AVRTask.o: ./tick.hpp ./strtoken.hpp ./gevent.hpp ./event.hpp ./simul.hpp
AVRTask.o: ./basestat.hpp ./debugstream.hpp ./particle.hpp ./plist.hpp
AVRTask.o: ./trace.hpp ./randomvar.hpp ./cloneable.hpp ./abstask.hpp
AVRTask.o: ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp
AVRTask.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./taskevt.hpp
AVRTask.o: ./feedback.hpp ./taskexc.hpp ./regvar.hpp ./factory.hpp
AVRTask.o: ./instr.hpp
basestat.o: ./baseexc.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
basestat.o: ./strtoken.hpp ./simul.hpp ./debugstream.hpp ./entity.hpp
basestat.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp
bwi.o: bwi.hpp resmanager.hpp ./entity.hpp ./baseexc.hpp ./basetype.hpp
bwi.o: ./tick.hpp ./strtoken.hpp ./scheduler.hpp ./plist.hpp ./simul.hpp
bwi.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
bwi.o: ./trace.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
bwi.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp server.hpp ./abskernel.hpp
bwi.o: ./task.hpp ./randomvar.hpp ./cloneable.hpp ./kernel.hpp ./kernevt.hpp
bwi.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp resource.hpp
capacitytimer.o: ./capacitytimer.hpp ./simul.hpp ./basestat.hpp
capacitytimer.o: ./basetype.hpp ./tick.hpp ./baseexc.hpp ./strtoken.hpp
capacitytimer.o: ./debugstream.hpp ./entity.hpp ./event.hpp ./particle.hpp
capacitytimer.o: ./plist.hpp ./trace.hpp ./config.hpp
cbserver.o: cbserver.hpp ./config.hpp server.hpp ./baseexc.hpp ./entity.hpp
cbserver.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
cbserver.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
cbserver.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
cbserver.o: ./abstask.hpp ./cpu.hpp ./timer.hpp ./powermodel.hpp
cbserver.o: ./minheap.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
cbserver.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./feedback.hpp
cbserver.o: ./taskexc.hpp ./scheduler.hpp resmanager.hpp ./capacitytimer.hpp
cbserver.o: ./exeinstr.hpp ./instr.hpp ./rttask.hpp ./regvar.hpp
cbserver.o: energyMRTKernel.hpp mrtkernel.hpp multi_cores_scheds.hpp
cbserver.o: ./rrsched.hpp ./edfsched.hpp ./ps_trace.hpp
cpu.o: ./cpu.hpp ./config.hpp ./trace.hpp ./baseexc.hpp ./basetype.hpp
cpu.o: ./tick.hpp ./strtoken.hpp ./timer.hpp ./simul.hpp ./basestat.hpp
cpu.o: ./debugstream.hpp ./entity.hpp ./event.hpp ./particle.hpp ./plist.hpp
cpu.o: ./gevent.hpp ./powermodel.hpp ./abstask.hpp ./minheap.hpp
cpu.o: energyMRTKernel.hpp mrtkernel.hpp ./kernel.hpp ./abskernel.hpp
cpu.o: ./kernevt.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp ./taskevt.hpp
cpu.o: ./feedback.hpp ./taskexc.hpp ./rttask.hpp ./regvar.hpp
cpu.o: multi_cores_scheds.hpp ./exeinstr.hpp ./instr.hpp ./rrsched.hpp
cpu.o: ./scheduler.hpp ./edfsched.hpp cbserver.hpp server.hpp resmanager.hpp
cpu.o: ./capacitytimer.hpp ./tracepowerbl.hpp ./tracefreqbl.hpp
debugstream.o: ./debugstream.hpp ./basetype.hpp ./tick.hpp ./baseexc.hpp
debugstream.o: ./strtoken.hpp ./simul.hpp ./basestat.hpp ./entity.hpp
debugstream.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp
edfsched.o: ./edfsched.hpp ./scheduler.hpp ./baseexc.hpp ./plist.hpp
edfsched.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
edfsched.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
edfsched.o: ./particle.hpp ./trace.hpp ./abstask.hpp ./cpu.hpp ./config.hpp
edfsched.o: ./timer.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
edfsched.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./rttask.hpp
edfsched.o: ./regvar.hpp ./randomvar.hpp ./cloneable.hpp ./task.hpp
edfsched.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
energyMRTKernel.o: energyMRTKernel.hpp ./config.hpp mrtkernel.hpp
energyMRTKernel.o: ./kernel.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp
energyMRTKernel.o: ./tick.hpp ./strtoken.hpp ./abskernel.hpp ./abstask.hpp
energyMRTKernel.o: ./cpu.hpp ./trace.hpp ./timer.hpp ./simul.hpp
energyMRTKernel.o: ./basestat.hpp ./debugstream.hpp ./event.hpp
energyMRTKernel.o: ./particle.hpp ./plist.hpp ./gevent.hpp ./powermodel.hpp
energyMRTKernel.o: ./minheap.hpp ./kernevt.hpp ./task.hpp ./randomvar.hpp
energyMRTKernel.o: ./cloneable.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
energyMRTKernel.o: ./rttask.hpp ./regvar.hpp multi_cores_scheds.hpp
energyMRTKernel.o: ./exeinstr.hpp ./instr.hpp ./rrsched.hpp ./scheduler.hpp
energyMRTKernel.o: ./edfsched.hpp cbserver.hpp server.hpp resmanager.hpp
energyMRTKernel.o: ./capacitytimer.hpp
entity.o: ./entity.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
entity.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./event.hpp
entity.o: ./particle.hpp ./plist.hpp ./trace.hpp
event.o: ./entity.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
event.o: ./event.hpp ./simul.hpp ./basestat.hpp ./debugstream.hpp
event.o: ./particle.hpp ./plist.hpp ./trace.hpp
exeinstr.o: ./factory.hpp ./simul.hpp ./basestat.hpp ./basetype.hpp
exeinstr.o: ./tick.hpp ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp
exeinstr.o: ./entity.hpp ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp
exeinstr.o: ./cpu.hpp ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp
exeinstr.o: ./abstask.hpp ./minheap.hpp ./exeinstr.hpp ./randomvar.hpp
exeinstr.o: ./cloneable.hpp ./instr.hpp ./taskevt.hpp ./task.hpp ./kernel.hpp
exeinstr.o: ./abskernel.hpp ./kernevt.hpp ./feedback.hpp ./taskexc.hpp
exeinstr.o: energyMRTKernel.hpp mrtkernel.hpp ./rttask.hpp ./regvar.hpp
exeinstr.o: multi_cores_scheds.hpp ./rrsched.hpp ./scheduler.hpp
exeinstr.o: ./edfsched.hpp cbserver.hpp server.hpp resmanager.hpp
exeinstr.o: ./capacitytimer.hpp
fcfsresmanager.o: ./abskernel.hpp ./abstask.hpp ./basetype.hpp ./tick.hpp
fcfsresmanager.o: ./baseexc.hpp ./strtoken.hpp ./cpu.hpp ./config.hpp
fcfsresmanager.o: ./trace.hpp ./timer.hpp ./simul.hpp ./basestat.hpp
fcfsresmanager.o: ./debugstream.hpp ./entity.hpp ./event.hpp ./particle.hpp
fcfsresmanager.o: ./plist.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
fcfsresmanager.o: ./fcfsresmanager.hpp resmanager.hpp ./scheduler.hpp
fcfsresmanager.o: resource.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
fcfsresmanager.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./feedback.hpp
fcfsresmanager.o: ./taskexc.hpp
feedback.o: ./feedback.hpp ./entity.hpp ./baseexc.hpp ./basetype.hpp
feedback.o: ./tick.hpp ./strtoken.hpp ./task.hpp ./gevent.hpp ./event.hpp
feedback.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./particle.hpp
feedback.o: ./plist.hpp ./trace.hpp ./randomvar.hpp ./cloneable.hpp
feedback.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
feedback.o: ./minheap.hpp ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
feedback.o: ./taskevt.hpp ./taskexc.hpp
feedbacktest.o: ./feedbacktest.hpp ./feedback.hpp ./entity.hpp ./baseexc.hpp
feedbacktest.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./sparepot.hpp
feedbacktest.o: ./sporadicserver.hpp server.hpp ./simul.hpp ./basestat.hpp
feedbacktest.o: ./debugstream.hpp ./event.hpp ./particle.hpp ./plist.hpp
feedbacktest.o: ./trace.hpp ./gevent.hpp ./abskernel.hpp ./abstask.hpp
feedbacktest.o: ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
feedbacktest.o: ./minheap.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
feedbacktest.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./taskexc.hpp
feedbacktest.o: ./scheduler.hpp resmanager.hpp ./capacitytimer.hpp
feedbacktest.o: ./supervisor.hpp
fifosched.o: ./fifosched.hpp ./scheduler.hpp ./baseexc.hpp ./plist.hpp
fifosched.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
fifosched.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
fifosched.o: ./particle.hpp ./trace.hpp ./abstask.hpp ./cpu.hpp ./config.hpp
fifosched.o: ./timer.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
fifosched.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./factory.hpp
fileImporter.o: ./fileImporter.hpp energyMRTKernel.hpp ./config.hpp
fileImporter.o: mrtkernel.hpp ./kernel.hpp ./baseexc.hpp ./entity.hpp
fileImporter.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./abskernel.hpp
fileImporter.o: ./abstask.hpp ./cpu.hpp ./trace.hpp ./timer.hpp ./simul.hpp
fileImporter.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
fileImporter.o: ./plist.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
fileImporter.o: ./kernevt.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
fileImporter.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp ./rttask.hpp
fileImporter.o: ./regvar.hpp multi_cores_scheds.hpp ./exeinstr.hpp
fileImporter.o: ./instr.hpp ./rrsched.hpp ./scheduler.hpp ./edfsched.hpp
fileImporter.o: cbserver.hpp server.hpp resmanager.hpp ./capacitytimer.hpp
fpsched.o: ./fpsched.hpp ./scheduler.hpp ./baseexc.hpp ./plist.hpp
fpsched.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
fpsched.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
fpsched.o: ./particle.hpp ./trace.hpp ./abstask.hpp ./cpu.hpp ./config.hpp
fpsched.o: ./timer.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
fpsched.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
genericvar.o: ./genericvar.hpp ./randomvar.hpp ./baseexc.hpp ./cloneable.hpp
genericvar.o: ./strtoken.hpp
grubserver.o: ./grubserver.hpp server.hpp ./baseexc.hpp ./entity.hpp
grubserver.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
grubserver.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
grubserver.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
grubserver.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
grubserver.o: ./powermodel.hpp ./minheap.hpp ./task.hpp ./randomvar.hpp
grubserver.o: ./cloneable.hpp ./kernel.hpp ./kernevt.hpp ./taskevt.hpp
grubserver.o: ./feedback.hpp ./taskexc.hpp ./scheduler.hpp resmanager.hpp
grubserver.o: ./capacitytimer.hpp
instr.o: ./instr.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp ./tick.hpp
instr.o: ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
instr.o: ./debugstream.hpp ./particle.hpp ./plist.hpp ./trace.hpp
instr.o: ./randomvar.hpp ./cloneable.hpp ./taskevt.hpp ./abstask.hpp
instr.o: ./cpu.hpp ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp
instr.o: ./minheap.hpp
interrupt.o: ./randomvar.hpp ./baseexc.hpp ./cloneable.hpp ./simul.hpp
interrupt.o: ./basestat.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
interrupt.o: ./debugstream.hpp ./entity.hpp ./event.hpp ./particle.hpp
interrupt.o: ./plist.hpp ./trace.hpp ./interrupt.hpp ./gevent.hpp ./task.hpp
interrupt.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
interrupt.o: ./powermodel.hpp ./minheap.hpp ./kernel.hpp ./abskernel.hpp
interrupt.o: ./kernevt.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
json_trace.o: ./json_trace.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
json_trace.o: ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
json_trace.o: ./debugstream.hpp ./entity.hpp ./particle.hpp ./plist.hpp
json_trace.o: ./trace.hpp ./rttask.hpp ./regvar.hpp ./randomvar.hpp
json_trace.o: ./cloneable.hpp ./task.hpp ./gevent.hpp ./abstask.hpp ./cpu.hpp
json_trace.o: ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp
json_trace.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./taskevt.hpp
json_trace.o: ./feedback.hpp ./taskexc.hpp
jtrace.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp ./baseexc.hpp
jtrace.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
jtrace.o: ./particle.hpp ./plist.hpp ./trace.hpp server.hpp ./gevent.hpp
jtrace.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
jtrace.o: ./powermodel.hpp ./minheap.hpp ./task.hpp ./randomvar.hpp
jtrace.o: ./cloneable.hpp ./kernel.hpp ./kernevt.hpp ./taskevt.hpp
jtrace.o: ./feedback.hpp ./taskexc.hpp ./scheduler.hpp resmanager.hpp
jtrace.o: ./jtrace.hpp ./traceevent.hpp ./waitinstr.hpp ./factory.hpp
jtrace.o: ./instr.hpp
kernel.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp ./baseexc.hpp
kernel.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
kernel.o: ./particle.hpp ./plist.hpp ./trace.hpp ./cpu.hpp ./config.hpp
kernel.o: ./timer.hpp ./gevent.hpp ./powermodel.hpp ./abstask.hpp
kernel.o: ./minheap.hpp ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
kernel.o: resmanager.hpp ./scheduler.hpp ./task.hpp ./randomvar.hpp
kernel.o: ./cloneable.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
kernel.o: ./reginstr.hpp
kernevt.o: ./kernel.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp ./tick.hpp
kernevt.o: ./strtoken.hpp ./abskernel.hpp ./abstask.hpp ./cpu.hpp
kernevt.o: ./config.hpp ./trace.hpp ./timer.hpp ./simul.hpp ./basestat.hpp
kernevt.o: ./debugstream.hpp ./event.hpp ./particle.hpp ./plist.hpp
kernevt.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./kernevt.hpp
load.o: ./exeinstr.hpp ./debugstream.hpp ./basetype.hpp ./tick.hpp
load.o: ./baseexc.hpp ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
load.o: ./entity.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./randomvar.hpp
load.o: ./cloneable.hpp ./instr.hpp ./taskevt.hpp ./abstask.hpp ./cpu.hpp
load.o: ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
load.o: ./load.hpp ./task.hpp ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
load.o: ./feedback.hpp ./taskexc.hpp
mrtkernel.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
mrtkernel.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
mrtkernel.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./cpu.hpp
mrtkernel.o: ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp
mrtkernel.o: ./abstask.hpp ./minheap.hpp mrtkernel.hpp ./kernel.hpp
mrtkernel.o: ./abskernel.hpp ./kernevt.hpp ./task.hpp ./randomvar.hpp
mrtkernel.o: ./cloneable.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
mrtkernel.o: resmanager.hpp ./scheduler.hpp cbserver.hpp server.hpp
mrtkernel.o: ./capacitytimer.hpp ./exeinstr.hpp ./instr.hpp ./rttask.hpp
mrtkernel.o: ./regvar.hpp
multi_cores_scheds.o: multi_cores_scheds.hpp ./config.hpp ./entity.hpp
multi_cores_scheds.o: ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
multi_cores_scheds.o: mrtkernel.hpp ./kernel.hpp ./abskernel.hpp
multi_cores_scheds.o: ./abstask.hpp ./cpu.hpp ./trace.hpp ./timer.hpp
multi_cores_scheds.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp
multi_cores_scheds.o: ./event.hpp ./particle.hpp ./plist.hpp ./gevent.hpp
multi_cores_scheds.o: ./powermodel.hpp ./minheap.hpp ./kernevt.hpp ./task.hpp
multi_cores_scheds.o: ./randomvar.hpp ./cloneable.hpp ./taskevt.hpp
multi_cores_scheds.o: ./feedback.hpp ./taskexc.hpp ./exeinstr.hpp ./instr.hpp
multi_cores_scheds.o: ./rrsched.hpp ./scheduler.hpp ./edfsched.hpp
multi_cores_scheds.o: energyMRTKernel.hpp ./rttask.hpp ./regvar.hpp
multi_cores_scheds.o: cbserver.hpp server.hpp resmanager.hpp
multi_cores_scheds.o: ./capacitytimer.hpp
paper_sim.o: ./config.hpp mrtkernel.hpp ./kernel.hpp ./baseexc.hpp
paper_sim.o: ./entity.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
paper_sim.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./trace.hpp ./timer.hpp
paper_sim.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./event.hpp
paper_sim.o: ./particle.hpp ./plist.hpp ./gevent.hpp ./powermodel.hpp
paper_sim.o: ./minheap.hpp ./kernevt.hpp ./task.hpp ./randomvar.hpp
paper_sim.o: ./cloneable.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
paper_sim.o: ./edfsched.hpp ./scheduler.hpp ./jtrace.hpp ./traceevent.hpp
paper_sim.o: ./texttrace.hpp ./rttask.hpp ./regvar.hpp ./json_trace.hpp
paper_sim.o: ./tracefreqbl.hpp ./ps_trace.hpp ./tracepowerbl.hpp ./instr.hpp
paper_sim.o: energyMRTKernel.hpp multi_cores_scheds.hpp ./exeinstr.hpp
paper_sim.o: ./rrsched.hpp cbserver.hpp server.hpp resmanager.hpp
paper_sim.o: ./capacitytimer.hpp ./taskstat.hpp ./fileImporter.hpp
piresman.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
piresman.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
piresman.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp
piresman.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
piresman.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./piresman.hpp
piresman.o: resmanager.hpp ./scheduler.hpp resource.hpp
pollingserver.o: pollingserver.hpp server.hpp ./baseexc.hpp ./entity.hpp
pollingserver.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
pollingserver.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
pollingserver.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
pollingserver.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
pollingserver.o: ./powermodel.hpp ./minheap.hpp ./task.hpp ./randomvar.hpp
pollingserver.o: ./cloneable.hpp ./kernel.hpp ./kernevt.hpp ./taskevt.hpp
pollingserver.o: ./feedback.hpp ./taskexc.hpp ./scheduler.hpp resmanager.hpp
powermodel.o: ./powermodel.hpp ./cpu.hpp ./config.hpp ./trace.hpp
powermodel.o: ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
powermodel.o: ./timer.hpp ./simul.hpp ./basestat.hpp ./debugstream.hpp
powermodel.o: ./entity.hpp ./event.hpp ./particle.hpp ./plist.hpp
powermodel.o: ./gevent.hpp ./abstask.hpp ./minheap.hpp
ps_trace.o: ./ps_trace.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
ps_trace.o: ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
ps_trace.o: ./debugstream.hpp ./entity.hpp ./particle.hpp ./plist.hpp
ps_trace.o: ./trace.hpp ./rttask.hpp ./regvar.hpp ./randomvar.hpp
ps_trace.o: ./cloneable.hpp ./task.hpp ./gevent.hpp ./abstask.hpp ./cpu.hpp
ps_trace.o: ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp
ps_trace.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./taskevt.hpp
ps_trace.o: ./feedback.hpp ./taskexc.hpp cbserver.hpp server.hpp
ps_trace.o: ./scheduler.hpp resmanager.hpp ./capacitytimer.hpp ./exeinstr.hpp
ps_trace.o: ./instr.hpp
randomvar.o: ./randomvar.hpp ./baseexc.hpp ./cloneable.hpp ./simul.hpp
randomvar.o: ./basestat.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
randomvar.o: ./debugstream.hpp ./entity.hpp ./event.hpp ./particle.hpp
randomvar.o: ./plist.hpp ./trace.hpp ./factory.hpp ./regvar.hpp
reginstr.o: ./strtoken.hpp ./baseexc.hpp ./exeinstr.hpp ./debugstream.hpp
reginstr.o: ./basetype.hpp ./tick.hpp ./event.hpp ./simul.hpp ./basestat.hpp
reginstr.o: ./entity.hpp ./particle.hpp ./plist.hpp ./trace.hpp
reginstr.o: ./randomvar.hpp ./cloneable.hpp ./instr.hpp ./taskevt.hpp
reginstr.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./gevent.hpp
reginstr.o: ./powermodel.hpp ./minheap.hpp ./threinstr.hpp ./factory.hpp
reginstr.o: ./schedinstr.hpp ./waitinstr.hpp ./suspend_instr.hpp
reginstr.o: ./reginstr.hpp
regsched.o: ./strtoken.hpp ./baseexc.hpp ./factory.hpp ./fifosched.hpp
regsched.o: ./scheduler.hpp ./plist.hpp ./simul.hpp ./basestat.hpp
regsched.o: ./basetype.hpp ./tick.hpp ./debugstream.hpp ./entity.hpp
regsched.o: ./event.hpp ./particle.hpp ./trace.hpp ./abstask.hpp ./cpu.hpp
regsched.o: ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp
regsched.o: ./minheap.hpp ./fpsched.hpp ./edfsched.hpp ./rrsched.hpp
regtask.o: ./factory.hpp ./AVRTask.hpp ./task.hpp ./entity.hpp ./baseexc.hpp
regtask.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./gevent.hpp ./event.hpp
regtask.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./particle.hpp
regtask.o: ./plist.hpp ./trace.hpp ./randomvar.hpp ./cloneable.hpp
regtask.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
regtask.o: ./minheap.hpp ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
regtask.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp ./regvar.hpp
regtask.o: ./instr.hpp ./rttask.hpp ./reginstr.hpp
regvar.o: ./regvar.hpp ./randomvar.hpp ./baseexc.hpp ./cloneable.hpp
regvar.o: ./genericvar.hpp ./factory.hpp
resmanager.o: ./entity.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
resmanager.o: ./strtoken.hpp ./abskernel.hpp ./abstask.hpp ./cpu.hpp
resmanager.o: ./config.hpp ./trace.hpp ./timer.hpp ./simul.hpp ./basestat.hpp
resmanager.o: ./debugstream.hpp ./event.hpp ./particle.hpp ./plist.hpp
resmanager.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp resmanager.hpp
resmanager.o: ./scheduler.hpp resource.hpp
resource.o: ./abstask.hpp ./basetype.hpp ./tick.hpp ./baseexc.hpp
resource.o: ./strtoken.hpp ./cpu.hpp ./config.hpp ./trace.hpp ./timer.hpp
resource.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./entity.hpp
resource.o: ./event.hpp ./particle.hpp ./plist.hpp ./gevent.hpp
resource.o: ./powermodel.hpp ./minheap.hpp resource.hpp
rmsched.o: ./kernel.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp ./tick.hpp
rmsched.o: ./strtoken.hpp ./abskernel.hpp ./abstask.hpp ./cpu.hpp
rmsched.o: ./config.hpp ./trace.hpp ./timer.hpp ./simul.hpp ./basestat.hpp
rmsched.o: ./debugstream.hpp ./event.hpp ./particle.hpp ./plist.hpp
rmsched.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./kernevt.hpp
rmsched.o: ./rmsched.hpp ./fpsched.hpp ./scheduler.hpp
rrsched.o: ./rrsched.hpp ./baseexc.hpp ./plist.hpp ./simul.hpp ./basestat.hpp
rrsched.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./debugstream.hpp
rrsched.o: ./entity.hpp ./event.hpp ./particle.hpp ./trace.hpp ./gevent.hpp
rrsched.o: ./scheduler.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
rrsched.o: ./powermodel.hpp ./minheap.hpp ./task.hpp ./randomvar.hpp
rrsched.o: ./cloneable.hpp ./kernel.hpp ./abskernel.hpp ./kernevt.hpp
rrsched.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp energyMRTKernel.hpp
rrsched.o: mrtkernel.hpp ./rttask.hpp ./regvar.hpp multi_cores_scheds.hpp
rrsched.o: ./exeinstr.hpp ./instr.hpp ./edfsched.hpp cbserver.hpp server.hpp
rrsched.o: resmanager.hpp ./capacitytimer.hpp
rttask.o: ./rttask.hpp ./regvar.hpp ./randomvar.hpp ./baseexc.hpp
rttask.o: ./cloneable.hpp ./simul.hpp ./basestat.hpp ./basetype.hpp
rttask.o: ./tick.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
rttask.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./task.hpp
rttask.o: ./gevent.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
rttask.o: ./powermodel.hpp ./minheap.hpp ./kernel.hpp ./abskernel.hpp
rttask.o: ./kernevt.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
rttask.o: ./taskstat.hpp
schedinstr.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
schedinstr.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
schedinstr.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./kernel.hpp
schedinstr.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp
schedinstr.o: ./timer.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
schedinstr.o: ./kernevt.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
schedinstr.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp ./schedinstr.hpp
schedinstr.o: ./factory.hpp ./instr.hpp
schedpoints.o: ./schedpoints.hpp ./supervisor.hpp server.hpp ./baseexc.hpp
schedpoints.o: ./entity.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
schedpoints.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./event.hpp
schedpoints.o: ./particle.hpp ./plist.hpp ./trace.hpp ./gevent.hpp
schedpoints.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp
schedpoints.o: ./timer.hpp ./powermodel.hpp ./minheap.hpp ./task.hpp
schedpoints.o: ./randomvar.hpp ./cloneable.hpp ./kernel.hpp ./kernevt.hpp
schedpoints.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp ./scheduler.hpp
schedpoints.o: resmanager.hpp ./sporadicserver.hpp ./capacitytimer.hpp
schedrta.o: ./schedrta.hpp ./supervisor.hpp server.hpp ./baseexc.hpp
schedrta.o: ./entity.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
schedrta.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
schedrta.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
schedrta.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
schedrta.o: ./minheap.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
schedrta.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./feedback.hpp
schedrta.o: ./taskexc.hpp ./scheduler.hpp resmanager.hpp ./sporadicserver.hpp
schedrta.o: ./capacitytimer.hpp
scheduler.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
scheduler.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
scheduler.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp
scheduler.o: ./scheduler.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
scheduler.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./task.hpp
scheduler.o: ./randomvar.hpp ./cloneable.hpp ./kernel.hpp ./abskernel.hpp
scheduler.o: ./kernevt.hpp ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
server.o: ./factory.hpp server.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp
server.o: ./tick.hpp ./strtoken.hpp ./simul.hpp ./basestat.hpp
server.o: ./debugstream.hpp ./event.hpp ./particle.hpp ./plist.hpp
server.o: ./trace.hpp ./gevent.hpp ./abskernel.hpp ./abstask.hpp ./cpu.hpp
server.o: ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp ./task.hpp
server.o: ./randomvar.hpp ./cloneable.hpp ./kernel.hpp ./kernevt.hpp
server.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp ./scheduler.hpp
server.o: resmanager.hpp
simul.o: ./entity.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
simul.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./event.hpp
simul.o: ./particle.hpp ./plist.hpp ./trace.hpp
sparepot.o: ./sparepot.hpp ./sporadicserver.hpp server.hpp ./baseexc.hpp
sparepot.o: ./entity.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
sparepot.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
sparepot.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
sparepot.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
sparepot.o: ./minheap.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
sparepot.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./feedback.hpp
sparepot.o: ./taskexc.hpp ./scheduler.hpp resmanager.hpp ./capacitytimer.hpp
sparepot.o: ./supervisor.hpp
sporadicserver.o: ./sporadicserver.hpp server.hpp ./baseexc.hpp ./entity.hpp
sporadicserver.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
sporadicserver.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
sporadicserver.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
sporadicserver.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
sporadicserver.o: ./powermodel.hpp ./minheap.hpp ./task.hpp ./randomvar.hpp
sporadicserver.o: ./cloneable.hpp ./kernel.hpp ./kernevt.hpp ./taskevt.hpp
sporadicserver.o: ./feedback.hpp ./taskexc.hpp ./scheduler.hpp resmanager.hpp
sporadicserver.o: ./capacitytimer.hpp
strtoken.o: ./strtoken.hpp ./baseexc.hpp
supercbs.o: ./supercbs.hpp ./supervisor.hpp server.hpp ./baseexc.hpp
supercbs.o: ./entity.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp ./simul.hpp
supercbs.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
supercbs.o: ./plist.hpp ./trace.hpp ./gevent.hpp ./abskernel.hpp
supercbs.o: ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp ./powermodel.hpp
supercbs.o: ./minheap.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
supercbs.o: ./kernel.hpp ./kernevt.hpp ./taskevt.hpp ./feedback.hpp
supercbs.o: ./taskexc.hpp ./scheduler.hpp resmanager.hpp ./sporadicserver.hpp
supercbs.o: ./capacitytimer.hpp cbserver.hpp ./exeinstr.hpp ./instr.hpp
supercbs.o: ./rttask.hpp ./regvar.hpp
suspend_instr.o: ./kernel.hpp ./baseexc.hpp ./entity.hpp ./basetype.hpp
suspend_instr.o: ./tick.hpp ./strtoken.hpp ./abskernel.hpp ./abstask.hpp
suspend_instr.o: ./cpu.hpp ./config.hpp ./trace.hpp ./timer.hpp ./simul.hpp
suspend_instr.o: ./basestat.hpp ./debugstream.hpp ./event.hpp ./particle.hpp
suspend_instr.o: ./plist.hpp ./gevent.hpp ./powermodel.hpp ./minheap.hpp
suspend_instr.o: ./kernevt.hpp ./task.hpp ./randomvar.hpp ./cloneable.hpp
suspend_instr.o: ./taskevt.hpp ./feedback.hpp ./taskexc.hpp
suspend_instr.o: ./suspend_instr.hpp ./instr.hpp
task.o: ./regvar.hpp ./randomvar.hpp ./baseexc.hpp ./cloneable.hpp
task.o: ./factory.hpp ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
task.o: ./strtoken.hpp ./debugstream.hpp ./entity.hpp ./event.hpp
task.o: ./particle.hpp ./plist.hpp ./trace.hpp ./abskernel.hpp ./abstask.hpp
task.o: ./cpu.hpp ./config.hpp ./timer.hpp ./gevent.hpp ./powermodel.hpp
task.o: ./minheap.hpp ./instr.hpp ./taskevt.hpp ./task.hpp ./kernel.hpp
task.o: ./kernevt.hpp ./feedback.hpp ./taskexc.hpp ./exeinstr.hpp
task.o: multi_cores_scheds.hpp mrtkernel.hpp ./rrsched.hpp ./scheduler.hpp
task.o: ./edfsched.hpp
taskevt.o: ./task.hpp ./entity.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
taskevt.o: ./strtoken.hpp ./gevent.hpp ./event.hpp ./simul.hpp ./basestat.hpp
taskevt.o: ./debugstream.hpp ./particle.hpp ./plist.hpp ./trace.hpp
taskevt.o: ./randomvar.hpp ./cloneable.hpp ./abstask.hpp ./cpu.hpp
taskevt.o: ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp
taskevt.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./taskevt.hpp
taskevt.o: ./feedback.hpp ./taskexc.hpp
texttrace.o: ./texttrace.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
texttrace.o: ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
texttrace.o: ./debugstream.hpp ./entity.hpp ./particle.hpp ./plist.hpp
texttrace.o: ./trace.hpp ./rttask.hpp ./regvar.hpp ./randomvar.hpp
texttrace.o: ./cloneable.hpp ./task.hpp ./gevent.hpp ./abstask.hpp ./cpu.hpp
texttrace.o: ./config.hpp ./timer.hpp ./powermodel.hpp ./minheap.hpp
texttrace.o: ./kernel.hpp ./abskernel.hpp ./kernevt.hpp ./taskevt.hpp
texttrace.o: ./feedback.hpp ./taskexc.hpp
threinstr.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
threinstr.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
threinstr.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./kernel.hpp
threinstr.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
threinstr.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./kernevt.hpp
threinstr.o: ./task.hpp ./randomvar.hpp ./cloneable.hpp ./taskevt.hpp
threinstr.o: ./feedback.hpp ./taskexc.hpp ./threinstr.hpp ./factory.hpp
threinstr.o: ./instr.hpp
tick.o: ./strtoken.hpp ./baseexc.hpp ./tick.hpp
timer.o: ./timer.hpp ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
timer.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
timer.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./gevent.hpp
trace.o: ./event.hpp ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
trace.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
trace.o: ./particle.hpp ./plist.hpp ./trace.hpp
traceevent.o: ./jtrace.hpp ./baseexc.hpp ./basetype.hpp ./tick.hpp
traceevent.o: ./strtoken.hpp ./event.hpp ./simul.hpp ./basestat.hpp
traceevent.o: ./debugstream.hpp ./entity.hpp ./particle.hpp ./plist.hpp
traceevent.o: ./trace.hpp ./traceevent.hpp
tracefreqbl.o: ./tracefreqbl.hpp ./cpu.hpp ./config.hpp ./trace.hpp
tracefreqbl.o: ./baseexc.hpp ./basetype.hpp ./tick.hpp ./strtoken.hpp
tracefreqbl.o: ./timer.hpp ./simul.hpp ./basestat.hpp ./debugstream.hpp
tracefreqbl.o: ./entity.hpp ./event.hpp ./particle.hpp ./plist.hpp
tracefreqbl.o: ./gevent.hpp ./powermodel.hpp ./abstask.hpp ./minheap.hpp
tracepower.o: tracepower.hpp ./cpu.hpp ./config.hpp ./trace.hpp ./baseexc.hpp
tracepower.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./timer.hpp
tracepower.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./entity.hpp
tracepower.o: ./event.hpp ./particle.hpp ./plist.hpp ./gevent.hpp
tracepower.o: ./powermodel.hpp ./abstask.hpp ./minheap.hpp
tracepowerbl.o: ./cpu.hpp ./config.hpp ./trace.hpp ./baseexc.hpp
tracepowerbl.o: ./basetype.hpp ./tick.hpp ./strtoken.hpp ./timer.hpp
tracepowerbl.o: ./simul.hpp ./basestat.hpp ./debugstream.hpp ./entity.hpp
tracepowerbl.o: ./event.hpp ./particle.hpp ./plist.hpp ./gevent.hpp
tracepowerbl.o: ./powermodel.hpp ./abstask.hpp ./minheap.hpp
tracepowerbl.o: ./tracepowerbl.hpp
waitinstr.o: ./simul.hpp ./basestat.hpp ./basetype.hpp ./tick.hpp
waitinstr.o: ./baseexc.hpp ./strtoken.hpp ./debugstream.hpp ./entity.hpp
waitinstr.o: ./event.hpp ./particle.hpp ./plist.hpp ./trace.hpp ./kernel.hpp
waitinstr.o: ./abskernel.hpp ./abstask.hpp ./cpu.hpp ./config.hpp ./timer.hpp
waitinstr.o: ./gevent.hpp ./powermodel.hpp ./minheap.hpp ./kernevt.hpp
waitinstr.o: ./task.hpp ./randomvar.hpp ./cloneable.hpp ./taskevt.hpp
waitinstr.o: ./feedback.hpp ./taskexc.hpp ./waitinstr.hpp ./factory.hpp
waitinstr.o: ./instr.hpp
