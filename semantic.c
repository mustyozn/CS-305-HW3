#include "semantic.h"
#include<stdio.h>//printf, sscanf
#include<stdlib.h>
#include<string.h>
#include <stdbool.h>//for bool



Date parseDate(char *dateStr, int line){
    Date d;
    d.line = line;
    sscanf(dateStr, "%d.%d.%d", &d.day, &d.month, &d.year);//extract day, month, year
    return d;//return the date object
}

bool isValidDate(Date d){
    //apply the rule 2 in the homework document for validation
    return (d.day >= 1 && d.day <= 28) && (d.month >= 1 && d.month <= 12) && (d.year >= 2025 && d.year <= 2050);
}


int compareDate(Date d1, Date d2) {
    //returns -1 if d1<d2, 0 if d1=d2, 1 if d1>d2
    if (d1.year != d2.year) return (d1.year > d2.year) ? 1 : -1;
    if (d1.month != d2.month) return (d1.month > d2.month) ? 1 : -1;
    if (d1.day != d2.day) return (d1.day > d2.day) ? 1 : -1;

    return 0;
}

int compareTime(Time t1, Time t2){
    if(t1.hour != t2.hour) return (t1.hour > t2.hour) ? 1 : -1;
    if(t1.minute != t2.minute) return (t1.minute > t2.minute) ? 1 : -1;
    return 0;
}

bool isValidTime(Time t){
    //apply rule 1 in the homework document for validation
    return (t.minute >= 0 && t.minute <= 59) && (t.hour >= 0 && t.hour <= 23);//return 0 if valid,return 1 otherwise
}

Time parseTime(char *timeStr, int line){
    Time t;
    t.line = line;
    sscanf(timeStr, "%d.%d", &t.hour, &t.minute);
    return t;
}

ParentStack parentStack;

void initStack() {
    parentStack.capacity = 8;
    parentStack.size = 0;
    parentStack.data = malloc(parentStack.capacity * sizeof(MeetingDetails*));
}

void pushParent(MeetingDetails* m) {
    if (parentStack.size == parentStack.capacity) {
        parentStack.capacity *= 2;
        parentStack.data = realloc(parentStack.data, parentStack.capacity * sizeof(MeetingDetails*));
    }
    parentStack.data[parentStack.size++] = m;
}

void popParent() {
    if (parentStack.size > 0) {
        parentStack.size--;
    }
}

MeetingDetails* currentParent() {
    return (parentStack.size > 0) ? parentStack.data[parentStack.size - 1] : NULL;
}

void freeStack() {
    free(parentStack.data);
}

TreeNode* createNode(MeetingDetails det) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->meeting = det;
    node->children = malloc(4 * sizeof(TreeNode*));
    node->childCapacity = 4;
    node->childCount = 0;
    return node;
}

void addChild(TreeNode *parent, TreeNode *child) {
    if (parent->childCount >= parent->childCapacity) {
        parent->childCapacity *= 2;
        parent->children = realloc(parent->children, parent->childCapacity * sizeof(TreeNode*));
    }
    parent->children[parent->childCount++] = child;
}

// Tree forest for top-level meetings
TreeNode **forestRoots = NULL;
int forestRootCount = 0;
int forestRootCapacity = 0;
bool allSemanticsValid = true;  // global default: assume no errors


void addMeetingToForest(TreeNode *node) {
    if (!forestRoots) {
        forestRootCapacity = 4;
        forestRoots = malloc(forestRootCapacity * sizeof(TreeNode*));
        forestRootCount = 0;
    }

    if (forestRootCount >= forestRootCapacity) {
        forestRootCapacity *= 2;
        forestRoots = realloc(forestRoots, forestRootCapacity * sizeof(TreeNode*));
    }

    forestRoots[forestRootCount++] = node;
}

void traverseTree(TreeNode *root) {
    if (!root) return;

    printf("Meeting #%d\n", root->meeting.meetingNumber);
    for (int i = 0; i < root->childCount; i++) {
        traverseTree(root->children[i]);
    }
}

void traverseTreeIndented(TreeNode *root, int level) {
    if (!root) return;
    for (int i = 0; i < level; ++i) printf("  ");
    printf("Meeting #%d\n", root->meeting.meetingNumber);
    for (int i = 0; i < root->childCount; ++i) {
        traverseTreeIndented(root->children[i], level + 1);
    }
}

MeetingNumberSet seenMeetingSet;
static TreeNode **treeStack = NULL;
static int treeStackSize = 0;
static int treeStackCapacity = 0;

void initTreeStack() {
    treeStackCapacity = 8;
    treeStackSize = 0;
    treeStack = malloc(treeStackCapacity * sizeof(TreeNode*));
}

void pushTreeNode(TreeNode *node) {
    if (treeStackSize == treeStackCapacity) {
        treeStackCapacity *= 2;
        treeStack = realloc(treeStack, treeStackCapacity * sizeof(TreeNode*));
    }
    treeStack[treeStackSize++] = node;
}

void popTreeNode() {
    if (treeStackSize > 0) {
        treeStackSize--;
    }
}

TreeNode* currentTreeParent() {
    return (treeStackSize > 0) ? treeStack[treeStackSize - 1] : NULL;
}

void freeTreeStack() {
    free(treeStack);
}

bool checkValidDate(Date d) {
    if (!isValidDate(d)) {
        reportInvalidDate(d);
        return false;
    }
    return true;
}

bool checkValidTime(Time t) {
    if (!isValidTime(t)) {
        reportInvalidTime(t);
        return false;
    }
    return true;
}

bool checkMeetingOrder(MeetingDetails m) {
    int cmpDate = compareDate(m.startDate, m.endDate);
    int cmpTime = compareTime(m.startTime, m.endTime);
    if (cmpDate > 0 || (cmpDate == 0 && cmpTime >= 0)) {
        reportEndTimeError(m);
        return false;
    }
    return true;
}

bool checkMeetingWithinParent(MeetingDetails sub, MeetingDetails parent) {
    bool startsTooEarly = compareDate(sub.startDate, parent.startDate) < 0 ||
        (compareDate(sub.startDate, parent.startDate) == 0 && compareTime(sub.startTime, parent.startTime) < 0);

    bool endsTooLate = compareDate(sub.endDate, parent.endDate) > 0 ||
        (compareDate(sub.endDate, parent.endDate) == 0 && compareTime(sub.endTime, parent.endTime) > 0);

    if (startsTooEarly || endsTooLate) {
        reportRangeError(sub, parent);
        return false;
    }
    return true;
}

// Reporting functions
void reportInvalidDate(Date d) {
    char msg[128];
    sprintf(msg, "%d_INVALID_DATE_(%02d.%02d.%04d)", d.line, d.day, d.month, d.year);
    addError(d.line, msg);
}

void reportInvalidTime(Time t) {
    char msg[128];
    sprintf(msg, "%d_INVALID_TIME_(%02d.%02d)", t.line, t.hour, t.minute);
    addError(t.line, msg);
}

void reportEndTimeError(MeetingDetails m) {
    char msg[128];
    sprintf(msg, "%d_ENDTIME_ERROR_(%d)", m.line, m.meetingNumber);
    addError(m.line, msg);
}

void reportRangeError(MeetingDetails sub, MeetingDetails parent) {
    char msg[128];
    sprintf(msg, "%d_RANGE_ERROR_(%d_%d)", sub.line, sub.meetingNumber, parent.meetingNumber);
    addError(sub.line, msg);
}


void initLocations(MeetingDetails* m) {
    m->locationCapacity = 4;
    m->locationCount = 0;
    m->locations = malloc(sizeof(char*) * m->locationCapacity);
}

void addLocation(MeetingDetails* m, const char* loc) {
    if (m->locationCount == m->locationCapacity) {
        m->locationCapacity *= 2;
        m->locations = realloc(m->locations, sizeof(char*) * m->locationCapacity);
    }
    m->locations[m->locationCount++] = strdup(loc); // strdup to make a copy
}

void freeLocations(MeetingDetails* m) {
    for (int i = 0; i < m->locationCount; ++i) {
        free(m->locations[i]);
    }
    free(m->locations);
    m->locations = NULL;
    m->locationCount = 0;
    m->locationCapacity = 0;
}

void checkRepeatedRooms(MeetingDetails m) {
    for (int i = 0; i < m.locationCount; ++i) {
        for (int j = i + 1; j < m.locationCount; ++j) {
            if (strcmp(m.locations[i], m.locations[j]) == 0) {
                // Only print once, even if multiple duplicates
                reportRepeatedRoomError(m);
                return;
            }
        }
    }
}

void reportRepeatedRoomError(MeetingDetails m) {
    char msg[512];
    int offset = sprintf(msg, "%d_REPEATED_ROOM_ERROR_(", m.locationsLine);
    for (int i = 0; i < m.locationCount; ++i) {
        offset += sprintf(msg + offset, "%s", m.locations[i]);
        if (i != m.locationCount - 1)
            offset += sprintf(msg + offset, ", ");
    }
    sprintf(msg + offset, ")");
    addError(m.locationsLine, msg);
}


void checkLocationInheritance(MeetingDetails sub, MeetingDetails parent) {
    for (int i = 0; i < sub.locationCount; ++i) {
        bool found = false;
        for (int j = 0; j < parent.locationCount; ++j) {
            if (strcmp(sub.locations[i], parent.locations[j]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            reportLocationInheritanceError(sub, parent);
            return;  // Only one error per submeeting, stop checking
        }
    }
}

void reportLocationInheritanceError(MeetingDetails sub, MeetingDetails parent) {
    char msg[128];
    sprintf(msg, "%d_LOCATION_ERROR_(%d_%d)", sub.locationsLine, sub.meetingNumber, parent.meetingNumber);
    addError(sub.locationsLine, msg);
}

bool isMeetingNumberSeen(int meetingNumber) {
    for (int i = 0; i < seenMeetingSet.size; ++i) {
        if (seenMeetingSet.meetingNumbers[i] == meetingNumber) {
            return true;
        }
    }
    return false;
}

void insertMeetingNumber(int meetingNumber) {
    if (seenMeetingSet.size == seenMeetingSet.capacity) {
        seenMeetingSet.capacity *= 2;
        seenMeetingSet.meetingNumbers = realloc(seenMeetingSet.meetingNumbers, sizeof(int) * seenMeetingSet.capacity);
    }
    seenMeetingSet.meetingNumbers[seenMeetingSet.size++] = meetingNumber;
}

void initMeetingNumberSet() {
    seenMeetingSet.capacity = 8;
    seenMeetingSet.size = 0;
    seenMeetingSet.meetingNumbers = malloc(sizeof(int) * seenMeetingSet.capacity);
}

void freeMeetingNumberSet() {
    free(seenMeetingSet.meetingNumbers);
    seenMeetingSet.meetingNumbers = NULL;
    seenMeetingSet.size = 0;
    seenMeetingSet.capacity = 0;
}


ErrorEntry* errorList = NULL;
int errorCount = 0;
int errorCapacity = 0;

//add error to the error list
void addError(int line, const char* message) {
    if (errorCount == errorCapacity) {
        errorCapacity = errorCapacity == 0 ? 8 : errorCapacity * 2;
        errorList = realloc(errorList, sizeof(ErrorEntry) * errorCapacity);//double the capacity if it is full, and allocate enough memory
    }

    errorList[errorCount].line = line;
    errorList[errorCount].message = strdup(message);  // copy message
    errorCount++;
    allSemanticsValid = false;//this is the logic to set the semantics to false
}


//this is the logic to be used to sort the images
int compareErrors(const void* a, const void* b) {
    const ErrorEntry* e1 = (const ErrorEntry*)a;
    const ErrorEntry* e2 = (const ErrorEntry*)b;

    if (e1->line != e2->line)
        return e1->line - e2->line;

    return strcmp(e1->message, e2->message);
}

//this is the logic to print the errors in sorted format
void printErrorsSorted() {
    qsort(errorList, errorCount, sizeof(ErrorEntry), compareErrors);

    for (int i = 0; i < errorCount; ++i) {
        printf("%s\n", errorList[i].message);
        free(errorList[i].message);  // cleanup(because once we print, we do not need it anymore)
    }
    free(errorList);
    errorList = NULL;
    errorCount = 0;
    errorCapacity = 0;
}




void validateRules(TreeNode* root, MeetingDetails* parent) {
    if (!root) return;

    MeetingDetails m = root->meeting;

    if (isMeetingNumberSeen(m.meetingNumber)) {
        //allSemanticsValid = false;
        char msg1[128];
        sprintf(msg1, "%d_REPEATED_MEETINGNUMBER_(%d)", m.meetingNumberLine, m.meetingNumber);
        addError(m.meetingNumberLine, msg1);
    } else {
        insertMeetingNumber(m.meetingNumber);//else insert it to the meeting Number Set (that holds all the meeting numbers as a set)
    }

    bool validStartDate = checkValidDate(m.startDate);
    bool validEndDate   = checkValidDate(m.endDate);
    bool validStartTime = checkValidTime(m.startTime);
    bool validEndTime   = checkValidTime(m.endTime);
    //rule 1-2
    bool orderValid = false;
    if (validStartDate && validEndDate && validStartTime && validEndTime) {
        orderValid = checkMeetingOrder(m);//checking rule 3
    }

    //checking rule 4
    if (parent && validStartDate && validEndDate && validStartTime && validEndTime && orderValid) {
        bool parentValid = isValidDate(parent->startDate) && isValidDate(parent->endDate) &&
                           isValidTime(parent->startTime) && isValidTime(parent->endTime);
    
        bool parentOrderValid = compareDate(parent->startDate, parent->endDate) < 0 ||
                                (compareDate(parent->startDate, parent->endDate) == 0 &&
                                 compareTime(parent->startTime, parent->endTime) < 0);
    
        if (parentValid && parentOrderValid) {
            checkMeetingWithinParent(m, *parent);
        }
    }
    

    //rule 6
    if(parent){
        checkLocationInheritance(m, *parent);

    }
    
    //checking rule 5
    checkRepeatedRooms(m);

    // Rule 7: Submeeting must not be recurring
    if (parent && m.isRecurring) {
        //allSemanticsValid = false;
        char msg2[128];
        sprintf(msg2, "%d_REPEATING_SUBMEETING_(%d)", m.isRecurringLine, m.meetingNumber);
        addError(m.isRecurringLine, msg2);
    }

    //Rule 8
    if (!m.isRecurring && m.hasFrequency) {
        //allSemanticsValid = false;
        char msg3[128];
        sprintf(msg3, "%d_UNEXPECTED_FREQUENCY_(%d)", m.frequencyLine, m.meetingNumber);
        addError(m.frequencyLine, msg3);
    }
    //Rule 9
    if (!m.isRecurring && m.hasRepetitionCount) {
        //allSemanticsValid = false;
        char msg4[128];
        sprintf(msg4, "%d_UNEXPECTED_REPETITIONCOUNT_(%d)", m.repetitionCountLine, m.meetingNumber);
        addError(m.repetitionCountLine, msg4);
    }
    
    // Rule 10: If recurring == yes, then both frequency and repetitionCount must exist(Do not forget, it says top level meeting block!)
    if (!parent && m.isRecurring) {
        if (!m.hasFrequency || !m.hasRepetitionCount) {
            allSemanticsValid = false;
            char msg5[128];
            sprintf(msg5, "%d_MISSING_ELEMENT_(%d)", m.isRecurringLine, m.meetingNumber);
            addError(m.isRecurringLine, msg5);
        }
    }

    
    //recursively check the submeetings
    for (int i = 0; i < root->childCount; ++i) {
        validateRules(root->children[i], &m);
    }
}

void collectLeafMeetings(TreeNode* node, ReportEntry** entries, int* count, int* capacity) {
    if (!node) return;

    // If it's a leaf node (no children), generate one ReportEntry per room
    if (node->childCount == 0) {
        MeetingDetails m = node->meeting;

        for (int i = 0; i < m.locationCount; ++i) {
            // Resize array if needed
            if (*count >= *capacity) {
                *capacity *= 2;//double the capacity
                *entries = realloc(*entries, (*capacity) * sizeof(ReportEntry));//allocate 
                if (*entries == NULL) return;  //we hopefully will not enter here, 
            }

            ReportEntry entry;
            entry.room = strdup(m.locations[i]);  // Allocate new string
            //set the dates and times
            entry.startDate = m.startDate;
            entry.startTime = m.startTime;
            entry.endDate = m.endDate;
            entry.endTime = m.endTime;
            entry.meetingNumber = m.meetingNumber;

            (*entries)[(*count)++] = entry;//push the entry to the array, and also increment count for the next pushes
        }
    }

    // Recurse into children
    for (int i = 0; i < node->childCount; ++i) {
        collectLeafMeetings(node->children[i], entries, count, capacity);
    }
}

//a helper function to implement date shifting logic
Date shiftDateByDays(Date d, int days) {
    d.day += days;

    while (d.day > 28) {
        //we need to repeat until there is no carry
        d.day -= 28;
        d.month += 1;//update the month

        if (d.month > 12) {//update the year if needed
            d.month = 1;
            d.year += 1;
        }
    }

    return d;//return the date
}

Date shiftDate(Date d, int amount, const char* frequency) {
    //amount is the ith recurrence
    if (strcmp(frequency, "daily") == 0) {
        //if we are here it means frequency is daily
        return shiftDateByDays(d, amount);
    } else if (strcmp(frequency, "weekly") == 0) {
        //if we are here it means frequency is weekly
        return shiftDateByDays(d, 7 * amount);
    } else if (strcmp(frequency, "monthly") == 0) {
        //if we are here it means frequency is monthly
        d.month += amount;
        while (d.month > 12) {
            d.month -= 12;
            d.year += 1;
        }
        return d;
    } else if (strcmp(frequency, "yearly") == 0) {
        d.year += amount;
        return d;
    }

    // Unknown frequency (fallback: no change)
    return d;
}

// Helper to deep copy and shift a node recursively
TreeNode* cloneAndShift(TreeNode* original, int offset, const char* frequency) {
    MeetingDetails orig = original->meeting;
    MeetingDetails copy = orig;

    // Shift dates
    copy.startDate = shiftDate(orig.startDate, offset, frequency);//shift the start date
    copy.endDate = shiftDate(orig.endDate, offset, frequency);//shift the end date

    // Remove recurrence in the copy
    copy.isRecurring = false;
    copy.hasFrequency = false;
    copy.hasRepetitionCount = false;
    copy.frequency = NULL;
    copy.repetitionCount = 0;

    TreeNode* newNode = createNode(copy);
    //we need to recursively do this for all it's children
    for (int i = 0; i < original->childCount; ++i) {
        TreeNode* childCopy = cloneAndShift(original->children[i], offset, frequency);
        addChild(newNode, childCopy);
    }

    return newNode;
}

void expandRecurringMeetings(TreeNode* root, ReportEntry** entries, int* count, int* capacity) {
    MeetingDetails m = root->meeting;

    if (!m.isRecurring || !m.hasFrequency || !m.hasRepetitionCount) {
        // If not recurring, fallback to simple leaf collection
        collectLeafMeetings(root, entries, count, capacity);
        return;
    }

    for (int i = 0; i < m.repetitionCount; ++i) {
        TreeNode* instance = cloneAndShift(root, i, m.frequency);
        collectLeafMeetings(instance, entries, count, capacity);
        // No need to add to forest or keep it around — this is one-time usage
    }
}
//this is the comparison logic (mentioned in the homework document)
//this is implemented so that it will be used in quicksort as a comparison logic
int compareEntries(const void* a, const void* b) {
    const ReportEntry* r1 = (const ReportEntry*)a;
    const ReportEntry* r2 = (const ReportEntry*)b;

    int cmpRoom = strcmp(r1->room, r2->room);
    if (cmpRoom != 0) return cmpRoom;

    //if we are here it means the rooms are the same
    int cmpDate = compareDate(r1->startDate, r2->startDate);
    if (cmpDate != 0) return cmpDate;

    return compareTime(r1->startTime, r2->startTime);
}

//this is the printing logic described in the document
void printSchedule(ReportEntry* entries, int count) {
    if (count == 0) return;

    char* currentRoom = NULL;

    for (int i = 0; i < count; ++i) {
        ReportEntry e = entries[i];

        // If it's a new room, print the room header
        if (currentRoom == NULL || strcmp(currentRoom, e.room) != 0) {
            if (currentRoom != NULL) free(currentRoom);  // free previous
            currentRoom = strdup(e.room);
            printf("%s:\n", currentRoom);
        }

        printf("%02d.%02d.%04d_%02d.%02d_%02d.%02d.%04d_%02d.%02d_%d\n",
               e.startDate.day, e.startDate.month, e.startDate.year,
               e.startTime.hour, e.startTime.minute,
               e.endDate.day, e.endDate.month, e.endDate.year,
               e.endTime.hour, e.endTime.minute,
               e.meetingNumber);
    }

    if (currentRoom) free(currentRoom);
}

//this is the main function that handles all the logic
void generateMeetingReport() {
    int capacity = 32;
    int count = 0;
    ReportEntry* entries = malloc(capacity * sizeof(ReportEntry));

    for (int i = 0; i < forestRootCount; ++i) {
        TreeNode* root = forestRoots[i];
        if (root->meeting.isRecurring) {
            expandRecurringMeetings(root, &entries, &count, &capacity);
        } else {
            collectLeafMeetings(root, &entries, &count, &capacity);
        }
    }

    qsort(entries, count, sizeof(ReportEntry), compareEntries);//call quicksort to sort them, and the comapare function is compareEntries
    printSchedule(entries, count);

    // Free memory
    for (int i = 0; i < count; ++i) {
        free(entries[i].room);
    }
    free(entries);
}




