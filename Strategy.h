#ifndef strategy_h
#define strategy_h
struct Strategy;
typedef struct Strategy Strategy;
typedef struct OfResult OfResult;

#include "Hdb.h"
#include "Page.h"

typedef OfResult (*OfStrategy)(PDB pHdb,PPage page,const Package package);
typedef bool (*MergeStrategy)(PDB pHdb,PPage page1,PPage page2);

struct OfResult{
	#define OFT_NOT		  (0x00)
	#define OFT_Mandatory (0x01)
	#define OFT_Condition (0x02)
	int ofType;
	int remainValues;
};

struct Strategy{
	OfStrategy ofStrategy;
	MergeStrategy mergeStrategy;
};
Strategy* newStrategy(OfStrategy,MergeStrategy);
void initStrategy(Strategy*,OfStrategy,MergeStrategy);
OfResult defaultOfStrategy(PDB pHdb,PPage page,const Package package);
bool defaultMergeStrategy(PDB pHdb,PPage page1,PPage page2);
#endif
