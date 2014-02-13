#include "Strategy.h"
#include "Page.h"
#include "const.h"
#include <assert.h>


static OfResult DEFAULT_OFRESULT={0,0};
Strategy* newStrategy(OfStrategy ofStrategy,MergeStrategy mergeStrategy){
	Strategy* strategy;
	strategy=(Strategy*)malloc(sizeof(Strategy));
	if(strategy){
		if(ofStrategy)
			strategy->ofStrategy=ofStrategy;
		else
			strategy->ofStrategy=defaultOfStrategy;
		if(mergeStrategy)
			strategy->mergeStrategy=mergeStrategy;
		else
			strategy->mergeStrategy=defaultMergeStrategy;
	}
	return strategy;
}
void initStrategy(Strategy* strategy,OfStrategy ofStrategy,MergeStrategy mergeStrategy){
	assert(strategy);
	if(strategy){
		if(ofStrategy)
			strategy->ofStrategy=ofStrategy;
		else
			strategy->ofStrategy=defaultOfStrategy;
		if(mergeStrategy)
			strategy->mergeStrategy=mergeStrategy;
		else
			strategy->mergeStrategy=defaultMergeStrategy;
	}
}
/*1. freeSpace
  2. spaceRate will be added in furture
*/
OfResult defaultOfStrategy(PDB pHdb,PPage page,const Package package){
	OfResult ofResult=DEFAULT_OFRESULT;
	assert(page->pageType == PT_DATA);
	if(package.valueSize > DEFAULT_PAGE_SIZE / 11 * 6){
		if(package.valueSize > DEFAULT_OFPAGE_MAX_VALUE){
			int freeSpace=getFreeSpace(page);
			if(package.valueSize % DEFAULT_OFPAGE_MAX_VALUE < freeSpace){
				ofResult.remainValues = package.valueSize % DEFAULT_OFPAGE_MAX_VALUE;
			}
		}
		else{
			ofResult.remainValues=0;			
		}
		ofResult.ofType = OFT_Mandatory;
	}
	else if(package.valueSize < DEFAULT_PAGE_SIZE /11 * 2){
		ofResult.ofType = OFT_NOT;
		ofResult.remainValues=package.valueSize;
	}
	else{
		int freeSpace = getFreeSpace(page);
		if(freeSpace > package.valueSize){
			ofResult.remainValues = package.valueSize;
		}
		else{
			ofResult.remainValues = 0;
		}
		ofResult.ofType = OFT_Condition;
	}
	return ofResult;
}

bool defaultMergeStrategy(PDB pHdb,PPage page1,PPage page2){
	int freeSpaceOfPage1,freeSpaceOfPage2;
	assert(page1->pageType == PT_DATA && page2->pageType == PT_DATA);
	freeSpaceOfPage1 = getFreeSpace(page1);
	freeSpaceOfPage2 = getFreeSpace(page2);
	if(freeSpaceOfPage1 + freeSpaceOfPage2 < DEFAULT_PAGE_FREE_SPACE){
		return true;
	}
	else{
		return false;
	}
}

