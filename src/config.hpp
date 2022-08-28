#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <cstdint>

extern int64_t BEGTICK; /* define it in a cpp file (else compiling takes long) */

#define TIME 					SIMUL.getTime()
#define EASSERT(x, t) 				do { if (x == false) cerr << "t=" <<  TIME << " " << t->toString() << endl; assert(x); } while(0)

#ifdef DEBUG
	#define ECOUT(x) 			do { if (int64_t(TIME) >= BEGTICK) cout << x; 	} while(0)
	#define ECOUTD(x) 			do { if (int64_t(TIME) >= BEGTICK) cout << x; 	} while(0)
	#define EPRINTF(printf)	 	do { if (int64_t(TIME) >= BEGTICK) printf;	 	} while(0)
#else
	#define ECOUT(x) 			do { } while(0)
	#define ECOUTD(x) 			do { } while(0)
	#define EPRINTF(printf)		do { } while(0)
#endif

#endif
