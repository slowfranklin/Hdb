#ifndef string_builder_h
#define string_builder_h

typedef struct StringBuilder StringBuilder;
typedef struct StringBuilder* PStringBuilder;

struct StringBuilder{
	char* buffer;
	int capacity;
	int len;
};
PStringBuilder newStringBuilder();
void appendStringBuilder(StringBuilder*,char*);
void insertStringBuilder(StringBuilder*,int,char* );
void freeStringBuilder(StringBuilder*);
#endif
