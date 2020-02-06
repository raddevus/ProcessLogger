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

static char* btrieveFileName = (char*)"squaresAndSquareRoots.btr";
string exeInfoFileName = "exe.info";

#define MIN_X 0
#define MAX_X 255
#pragma pack(1)

int recordSize = 0;

typedef struct {
    uint8_t x;
    uint16_t xSquared;
    double xSquareRoot;
} record_t;

typedef struct {
    char exeName[51];
    char dateTime[20];
    char userName[51];
} executableInfo;

const int TOTAL_EXEINFO_LENGTH = 122;

#pragma pack()
// Disables a warning about using 
// localtime, for our sample code this is fine
#pragma warning(disable : 4996) 

typedef uint8_t _key_t;

string getDateTime() {
    auto t = std::time(nullptr);
    auto tm = *localtime(&t);
    std::ostringstream oss;
    oss << put_time(&tm, "%m-%d-%Y %H:%M:%S");
    return oss.str();
}

static Btrieve::StatusCode
createFile(BtrieveClient* btrieveClient, const char* fileName)
{
    Btrieve::StatusCode status;
    BtrieveFileAttributes btrieveFileAttributes;
    // If SetFixedRecordLength() fails.
    cout << getDateTime() << endl;
    if ((status = btrieveFileAttributes.SetFixedRecordLength(sizeof(record_t))) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFileAttributes::SetFixedRecordLength():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If FileCreate() fails.
    if ((status = btrieveClient->FileCreate(&btrieveFileAttributes, fileName, Btrieve::CREATE_MODE_OVERWRITE)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
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

    executableInfo record;
    sprintf(record.exeName,"%s",exeName.c_str());
    sprintf(record.dateTime, "%s",dateTime.c_str());
    sprintf(record.userName,"%s",userName.c_str());
    // If RecordCreate() fails.
    if ((status = btrieveFile->RecordCreate((char*)&record, TOTAL_EXEINFO_LENGTH)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFile::RecordCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    
leave:
    return status;
}


static Btrieve::StatusCode
loadFile(BtrieveFile* btrieveFile)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    int i;
    record_t record;
    for (i = MIN_X; i <= MAX_X; i++)
    {
        record.x = (uint8_t)i;
        record.xSquared = (uint16_t)(i * i);
        record.xSquareRoot = sqrt((double)i);
        // If RecordCreate() fails.
        if ((status = btrieveFile->RecordCreate((char*)&record, sizeof(record))) != Btrieve::STATUS_CODE_NO_ERROR)
        {
            printf("Error: BtrieveFile::RecordCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
            goto leave;
        }
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
deleteFile(BtrieveClient* btrieveClient)
{
    Btrieve::StatusCode status;
    // If FileDelete() fails.
    if ((status = btrieveClient->FileDelete(btrieveFileName)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileDelete():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
}
static Btrieve::StatusCode
createIndex(BtrieveFile* btrieveFile)
{
    Btrieve::StatusCode status;
    BtrieveIndexAttributes btrieveIndexAttributes;
    BtrieveKeySegment btrieveKeySegment;
    // If SetField() fails.
    if ((status = btrieveKeySegment.SetField(0, 1, Btrieve::DATA_TYPE_UNSIGNED_BINARY)) != Btrieve::STATUS_CODE_NO_ERROR)
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
retrieveRecord(BtrieveFile* btrieveFile, _key_t* key)
{
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    record_t record;
    // If RecordRetrieve() fails.
    if (btrieveFile->RecordRetrieve(Btrieve::COMPARISON_EQUAL, Btrieve::INDEX_1, (char*)key, sizeof(*key), (char*)&record, sizeof(record)) != sizeof(record))
    {
        status = btrieveFile->GetLastStatusCode();
        printf("Error: BtrieveFile::RecordRetrieve():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    printf("record: (%u, %u, %f)\n", record.x, record.xSquared, record.xSquareRoot);
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
    if ((status = btrieveFileAttributes.SetFixedRecordLength(TOTAL_EXEINFO_LENGTH)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFileAttributes::SetFixedRecordLength():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If FileCreate() fails.
    if ((status = btrieveClient->FileCreate(&btrieveFileAttributes, fileName, Btrieve::CREATE_MODE_OVERWRITE)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
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
    if ((status = btrieveKeySegment.SetField(0, 1, Btrieve::DATA_TYPE_LSTRING)) != Btrieve::STATUS_CODE_NO_ERROR)
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
    // If RecordRetrieve() fails.
    char keyCopy[51];

    strcpy(keyCopy, key.c_str());
    if (btrieveFile->RecordRetrieve(Btrieve::COMPARISON_EQUAL,
        Btrieve::INDEX_1, keyCopy, 
        key.length() *sizeof(char), 
        (char*)&record, TOTAL_EXEINFO_LENGTH ) != TOTAL_EXEINFO_LENGTH)//sizeof(record))
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

int main(int argc, char* argv[])
{

    if (argc < 3 || argc > 3) {
        cout << "### Must provide two arguments.  ####" << endl;
        cout << "Usage: $/>ProcessLogger [exe_name] [user_name]" << endl;
        return -1;
    }

    char* processName = argv[1];
    printf("%s\n", processName);

    char* user = argv[2];
    printf("%s\n", user);

    BtrieveClient btrieveClient(0x4232, 0);
    BtrieveFile btrieveFile;
    //BtrieveFile bFile;

    Btrieve::StatusCode status = Btrieve::STATUS_CODE_NO_ERROR;
    if ((status = createInfoFile(&btrieveClient, exeInfoFileName.c_str())) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    //If openFile() fails.
    if ((status = openFile(&btrieveClient, &btrieveFile, exeInfoFileName.c_str())) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }

    if ((status = createInfoIndex(&btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
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
