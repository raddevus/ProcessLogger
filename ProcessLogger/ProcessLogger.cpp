#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "include\btrieveCpp.h"
using namespace std;

// exe.info is the name of the file that will hold our data
string exeInfoFileName = "exe.info";

#pragma pack(1)

int recordSize = 0;

typedef struct {
    char exeName[51]; // data + 1 for '\0'
    char dateTime[20]; // data + 1 for '\0'
    char userName[51]; // data + 1 for '\0'
} executableInfo;

// This is the total executableInfo Record size in bytes.
const int TOTAL_EXEINFO_RECORD_LENGTH = 122;

#pragma pack()
// Disables a warning about using 
// localtime, for our sample code this is fine
#pragma warning(disable : 4996) 

string getDateTime() {
    // ##### Provides a simple function which returns a 19-byte
    // date-time formatted string for the current date-time
    // Format looks like : MM-DD-YYYY HH:MM:SS
    // Example :           02-06-2020 10:29:47
    auto t = std::time(nullptr);
    auto tm = *localtime(&t);
    std::ostringstream oss;
    oss << put_time(&tm, "%m-%d-%Y %H:%M:%S");
    return oss.str();
}

static Btrieve::StatusCode
openFile(BtrieveClient* btrieveClient, BtrieveFile* btrieveFile, const char* fileName)
{
    Btrieve::StatusCode status;
    // If FileOpen() fails.
    if ((status = btrieveClient->FileOpen(btrieveFile, fileName, NULL, Btrieve::OPEN_MODE_NORMAL)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileOpen():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
}

bool ValidateStructLengths(string exeName, string dateTime, string userName) {
    if (exeName.length() > 50) { printf("exeName value too long (>50 chars).\n");  return false; }
    if (dateTime.length() > 19) { printf("dateTime value too long (>19 chars).\n"); return false; }
    if (userName.length() > 50) { printf("userName value too long (>50 chars).\n"); return false; }
    recordSize = userName.length() + dateTime.length() + exeName.length();
    return true;
}

string ForceKeyLengthTo5(string key) {
    while (key.length() < 5) {
        key += key;
    }
    if (key.length() > 5) {
        key = key.substr(0, 5);
    }
    return key;
}

static Btrieve::StatusCode
addRecordToInfoFile(BtrieveFile* btrieveFile, string exeName, string dateTime, string userName)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;

    //Check the length of each item in executableInfo 
    if (!ValidateStructLengths(exeName,dateTime,userName)) {
        status = Btrieve::STATUS_CODE_DATALENGTH_ERROR;
        printf("Error: bad value! Cannot continue:%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    exeName = ForceKeyLengthTo5(exeName);
    executableInfo record;
    sprintf(record.exeName,"%s",exeName.c_str());
    sprintf(record.dateTime, "%s",dateTime.c_str());
    sprintf(record.userName,"%s",userName.c_str());
    // If RecordCreate() fails.
    if ((status = btrieveFile->RecordCreate((char*)&record, TOTAL_EXEINFO_RECORD_LENGTH)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFile::RecordCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    
leave:
    return status;
}

static Btrieve::StatusCode
closeFile(BtrieveClient* btrieveClient, BtrieveFile* btrieveFile)
{
    Btrieve::StatusCode status;
    // If FileClose() fails.
    if ((status = btrieveClient->FileClose(btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileClose():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
}

static Btrieve::StatusCode
createInfoFile(BtrieveClient* btrieveClient, const char* fileName)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;;
    BtrieveFileAttributes btrieveFileAttributes;
    // If SetFixedRecordLength() fails.
    cout << getDateTime() << endl;
    if ((status = btrieveFileAttributes.SetFixedRecordLength(TOTAL_EXEINFO_RECORD_LENGTH)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFileAttributes::SetFixedRecordLength():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If FileCreate() fails.
    if ((status = btrieveClient->FileCreate(&btrieveFileAttributes, fileName, Btrieve::CREATE_MODE_NO_OVERWRITE)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        if (status != Btrieve::STATUS_CODE_FILE_ALREADY_EXISTS) {
            printf("Error: BtrieveClient::FileCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
            goto leave;
        }
    }
leave:
    return status;
}

static Btrieve::StatusCode
createInfoIndex(BtrieveFile* btrieveFile)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    BtrieveIndexAttributes btrieveIndexAttributes;
    BtrieveKeySegment btrieveKeySegment;
    // If SetField() fails.
    if ((status = btrieveKeySegment.SetField(0, 5, Btrieve::DATA_TYPE_LSTRING)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveKeySegment::SetField():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If AddKeySegment() fails.
    if ((status = btrieveIndexAttributes.AddKeySegment(&btrieveKeySegment)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveIndexAttributes::AddKeySegment():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If IndexCreate() fails.
    if ((status = btrieveFile->IndexCreate(&btrieveIndexAttributes)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFile::IndexCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
}

static Btrieve::StatusCode
retrieveInfoRecord(BtrieveFile* btrieveFile, string key)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    executableInfo record;
    
    key = ForceKeyLengthTo5(key);
    
    // If RecordRetrieve() fails.
    if (btrieveFile->RecordRetrieve(Btrieve::COMPARISON_EQUAL,
        Btrieve::INDEX_1, key.c_str(), 
        key.size(), 
        (char*)&record, TOTAL_EXEINFO_RECORD_LENGTH ) != TOTAL_EXEINFO_RECORD_LENGTH)//sizeof(record))
    {
        status = btrieveFile->GetLastStatusCode();
        printf("Error: BtrieveFile::RecordRetrieve():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    status = btrieveFile->GetLastStatusCode();
    printf("record: (%s, %s, %s)\n", record.exeName, record.userName, record.dateTime);
    //printf("record: (%s, %s, %s)\n", record.exeName, record.dateTime, record.userName);
leave:
    return status;
}

Btrieve::StatusCode getRecord(char * processName) {
    BtrieveClient btrieveClient(0x4232, 0);
    BtrieveFile btrieveFile;

    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    //If openFile() fails.
    if ((status = openFile(&btrieveClient, &btrieveFile, exeInfoFileName.c_str())) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        return status;
    }
    if ((status = retrieveInfoRecord(&btrieveFile, processName)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        return status;
    }

    if ((status = closeFile(&btrieveClient, &btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        return status;
    }
    return status;

}

int main(int argc, char* argv[])
{

    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;

    
    if (argc == 2) {
        argv[1];
        status = getRecord(argv[1]);
        return 0;
    }

    if (argc < 2 || argc > 3) {
        cout << "### Must provide one or two arguments.  ####" << endl;
        cout << "Usage: Add record, $/>ProcessLogger [exe_name] [user_name]" << endl;
        cout << "Usage: Get record, $/>ProcessLogger [exe_name]" << endl;
        return -1;
    }

    
    char* processName = argv[1];
    printf("%s\n", processName);

    char* user = argv[2];
    printf("%s\n", user);

    BtrieveClient btrieveClient(0x4232, 0);
    BtrieveFile btrieveFile;
    //BtrieveFile bFile;
    
    if ((status = createInfoFile(&btrieveClient, exeInfoFileName.c_str())) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        if (status != Btrieve::STATUS_CODE_FILE_ALREADY_EXISTS) {
            goto leave;
        }
    }
    //If openFile() fails.
    if ((status = openFile(&btrieveClient, &btrieveFile, exeInfoFileName.c_str())) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }

    if ((status = createInfoIndex(&btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        if (status != Btrieve::STATUS_CODE_DUPLICATE_KEY_VALUE) {
            goto leave;
        }
    }

    if ((status = addRecordToInfoFile(&btrieveFile, processName, getDateTime(), user)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }

    if ((status = retrieveInfoRecord(&btrieveFile, processName)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }

    if ((status = closeFile(&btrieveClient, &btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }

leave:
    // If there wasn't a failure.
    if (status == Btrieve::STATUS_CODE_NO_ERROR)
        return 0;
    return 1;
}
