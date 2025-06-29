#ifndef __SEMANTIC_H
#define __SEMANTIC_H
#include <stdbool.h>


typedef enum {FREQ_DAILY, FREQ_WEEKLY, FREQ_MONTHLY, FREQ_YEARLY} frequencyType;


//simple data class 
typedef struct{
    int day;
    int month;
    int year;
    int line;//for error reporting
}Date;

typedef struct{ 
    int hour;
    int minute;
    int line;//for error reporting

}Time;

typedef struct{
    int meetingNumber;
    int line;
    //bool isSubmeeting;//this will be useful for rule 7

}MeetingInfo;



typedef struct {
    int meetingNumber;
    int line;
    int meetingNumberLine;  // this is neeeded for rule 11 where we need to display the duplicates
    Date startDate, endDate;
    Time startTime, endTime;
    bool dateTimeValid;

    // Dynamic location list
    char** locations;     // dynamic array of strings
    int locationCount;    // how many rooms are currently stored
    int locationCapacity; // current capacity of the array
    int locationsLine;    // line number for 'locations = ...'

    //new fields for rule 7
    bool isRecurring;
    int isRecurringLine;

    // new fields for rule 8
    bool hasFrequency;
    int frequencyLine;
    char* frequency;  // frequency, the frequency type, whether monthly, weekly, yearly and so ono



    //new fields for rule 9
    bool hasRepetitionCount;
    int repetitionCountLine;
    int repetitionCount; // repetition count, how often does it occur



} MeetingDetails;


typedef struct {
    MeetingDetails info;
    bool isSubmeeting;
    MeetingDetails* parentInfo;  // No self-referencing, just a pointer to parent data
} MeetingContext;


typedef struct {
    char* value;   // e.g., "weekly", "monthly", etc.
    int line;
} FrequencyInfo;

typedef struct {
    int value;     // repetitionCount as an integer
    int line;
} RepetitionInfo;



// Dynamic Stack of MeetingDetails*
typedef struct {
    MeetingDetails** data;
    int size;
    int capacity;
} ParentStack;

extern ParentStack parentStack;

void initStack();
void pushParent(MeetingDetails* m);
void popParent();
MeetingDetails* currentParent();
void freeStack();


typedef struct TreeNode {
    MeetingDetails meeting;
    struct TreeNode **children;
    int childCount;
    int childCapacity;
} TreeNode;

extern TreeNode **forestRoots;
extern int forestRootCount;
extern int forestRootCapacity;

TreeNode* createNode(MeetingDetails det);
void addChild(TreeNode *parent, TreeNode *child);
void addMeetingToForest(TreeNode *node);
void traverseTree(TreeNode *root);

// Tree stack operations for tree construction
void initTreeStack();
void pushTreeNode(TreeNode *node);
void popTreeNode();
TreeNode* currentTreeParent();
void freeTreeStack();

void traverseTreeIndented(TreeNode *root, int level);
void validateRules(TreeNode* root, MeetingDetails* parent);



Date parseDate(char *dateStr, int line);
int compareDate(Date d1, Date d2);//returns -1 if d1 < d2, returns 0 if d1=d2, returns 1 if d1 > d2
bool isValidDate(Date d);

Time parseTime(char* timeStr, int line);//inputs the time string(e.g. "13.30" and the line number needed)
int compareTime(Time t1, Time t2);
bool isValidTime(Time t);

// Individual semantic rule checks
bool checkValidDate(Date d);
bool checkValidTime(Time t);
bool checkMeetingOrder(MeetingDetails m);
bool checkMeetingWithinParent(MeetingDetails sub, MeetingDetails parent);

void reportInvalidDate(Date d);
void reportInvalidTime(Time t);
void reportEndTimeError(MeetingDetails m);
void reportRangeError(MeetingDetails sub, MeetingDetails parent);

void initLocations(MeetingDetails* m);
void addLocation(MeetingDetails* m, const char* loc);
void freeLocations(MeetingDetails* m);

void checkRepeatedRooms(MeetingDetails m);
void reportRepeatedRoomError(MeetingDetails m);

void checkLocationInheritance(MeetingDetails sub, MeetingDetails parent);
void reportLocationInheritanceError(MeetingDetails sub, MeetingDetails parent);

//this will be needed to keep the duplicates
typedef struct {
    int* meetingNumbers;  // more descriptive than `data`
    int size;
    int capacity;
} MeetingNumberSet;


extern MeetingNumberSet seenMeetingSet;
extern bool allSemanticsValid;


void initMeetingNumberSet();
void freeMeetingNumberSet();
bool isMeetingNumberSeen(int meetingNumber);
void insertMeetingNumber(int meetingNumber);



//This will hold the informations to put all these stuff to there. 
typedef struct {
    char* room;
    Date startDate;
    Time startTime;
    Date endDate;
    Time endTime;
    int meetingNumber;
} ReportEntry;

// ============================
// Report Generation Functions
// ============================

void generateMeetingReport();  // Master function to be called in main if allSemanticsValid

// Collect and expand logic
void collectLeafMeetings(TreeNode* node, ReportEntry** entries, int* count, int* capacity);
void expandRecurringMeetings(TreeNode* root, ReportEntry** entries, int* count, int* capacity);

// Date shifting utilities (planet CS305 logic)
Date shiftDateByDays(Date d, int days);
Date shiftDate(Date d, int amount, const char* frequency);  // frequency: daily/weekly/monthly/yearly

// Sorting and output
int compareEntries(const void* a, const void* b);
void printSchedule(ReportEntry* entries, int count);


typedef struct {
    int line;
    char* message;  // Full formatted message (e.g., "4_INVALID_DATE_(99.99.9999)")
} ErrorEntry;

//we need this to sort the error messages(because I had output variances with the expected output)
extern ErrorEntry* errorList;
extern int errorCount;
extern int errorCapacity;
// Error handling utilities
void addError(int line, const char* message);
void printErrorsSorted();












#endif