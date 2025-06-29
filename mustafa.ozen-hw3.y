%{
    #include "semantic.h"
    #include "semantic.c"
    #include <stdio.h>
    int yylex();
    void yyerror(const char *s) { }
%}
%locations


%union {
    int intVal;
    char* strVal;
    Time timeObj;
    Date dateObj;
    MeetingDetails meetingDetails;
    TreeNode* treeNodePtr;
    char* stringVal; 
    char** strList; 
    FrequencyInfo freq_info;
    RepetitionInfo rep_info;

}

%type <treeNodePtr> meeting
%type <treeNodePtr> meeting_list
%type <meetingDetails> features
%type <stringVal> tIDENTIFIER
%type <strList> locations
%type <intVal> bool_val
%type <freq_info> frequency
%type <rep_info>  repetition
%type <strVal> freq






%token tSTARTDATE tENDDATE tLOCATIONS  tDAILY tMONTHLY tWEEKLY tYEARLY
%token tYES tNO tSTARTSUBMEETINGS tENDSUBMEETINGS
%token tSTARTTIME tENDTIME tASSIGN tDESCRIPTION 
%token  tCOMMA tSTRING tIDENTIFIER tENDMEETING

%token <dateObj> tDATE
%token <timeObj> tTIME
%token <intVal> tMEETINGNUMBER
%token <intVal> tSTARTMEETING
%token <intVal> tINTEGER
%token <intVal> tISRECURRING
%token <intVal> tREPETITIONCOUNT
%token <intVal> tFREQUENCY



%%

program: meeting_list;

meeting_list:
    meeting                      { $$ = $1; }
    | meeting meeting_list       { $$ = $1; } // $$ not used since we handle tree connections inside

meeting:
    tSTARTMEETING features {
        $2.line = $1;
        TreeNode* node = createNode($2);
        TreeNode* parent = currentTreeParent();

        if (parent) {
            addChild(parent, node);
        } else {
            addMeetingToForest(node);
        }

        pushTreeNode(node);
    }
    submeet
    tENDMEETING {
        popTreeNode();
        $$ = NULL; // not used downstream
    };

features:
    tSTRING tMEETINGNUMBER tASSIGN tINTEGER
    tDESCRIPTION tASSIGN tSTRING
    tSTARTDATE tASSIGN tDATE
    tSTARTTIME tASSIGN tTIME
    tENDDATE tASSIGN tDATE
    tENDTIME tASSIGN tTIME
    tLOCATIONS tASSIGN locations
    tISRECURRING tASSIGN bool_val
    frequency repetition
    {
        MeetingDetails det;
        det.meetingNumber = $4;
        det.startDate = $10;
        det.startTime = $13;
        det.endDate   = $16;
        det.endTime   = $19;
        det.meetingNumberLine = @2.first_line;

        det.isRecurring = ($25 == 1);      
        det.isRecurringLine = $23;            

        det.locations = $22;                    
        det.locationCount = 0;
        while ($22[det.locationCount] != NULL)  
            det.locationCount++;
        det.locationCapacity = det.locationCount;
        det.locationsLine = @20.first_line;

        det.frequency = $26.value;
        det.frequencyLine = $26.line;
        det.hasFrequency = ($26.value != NULL);


        det.repetitionCount = $27.value;
        det.repetitionCountLine = $27.line;
        det.hasRepetitionCount = ($27.value != -1); 


        $$ = det;
    };

locations:
    tIDENTIFIER {
        $$ = malloc(sizeof(char*) * 2);
        $$[0] = $1;
        $$[1] = NULL;
    }
    | tIDENTIFIER tCOMMA locations {
        int count = 0;
        while ($3[count] != NULL) count++;

        $$ = malloc(sizeof(char*) * (count + 2));
        $$[0] = $1;
        for (int i = 0; i <= count; ++i)
            $$[i + 1] = $3[i];

        free($3);  
    };


bool_val:
    tYES { $$ = 1; }
    | tNO { $$ = 0; }

frequency:
    tFREQUENCY tASSIGN freq {
        FrequencyInfo fi;
        fi.value = $3;
        fi.line = $1; 
        $$ = fi;
    }
    | {
        FrequencyInfo fi;
        fi.value = NULL;
        fi.line = -1;
        $$ = fi;
    }
;


freq:
    tDAILY   { $$ = "daily";   }
    | tWEEKLY  { $$ = "weekly";  }
    | tMONTHLY { $$ = "monthly"; }
    | tYEARLY  { $$ = "yearly";  }
;


repetition:
    tREPETITIONCOUNT tASSIGN tINTEGER {
        RepetitionInfo ri;
        ri.value = $3;
        ri.line = $1;
        $$ = ri;
    }
    | {
        RepetitionInfo ri;
        ri.value = -1;
        ri.line = -1;
        $$ = ri;
    };

submeet:
    tSTARTSUBMEETINGS meeting_list tENDSUBMEETINGS
    |
    ;

%%

int main() {
    initMeetingNumberSet(); 
    initTreeStack();  
    initStack(); // Don't forget to init the parentStack too

    if (yyparse()) {
        printf("ERROR\n");
        return 1;
    }

    for (int i = 0; i < forestRootCount; ++i) {
        validateRules(forestRoots[i], NULL);  // Root has no parent
    }

    if (allSemanticsValid) {
        generateMeetingReport();
    } else {
        printErrorsSorted();  
    }

    // Final cleanup
    freeMeetingNumberSet(); 
    freeTreeStack();
    freeStack();

    return 0;
}

