#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>

#include "include\btrieveCpp.h"
using namespace std;

static char* btrieveFileName = (char*)"squaresAndSquareRoots.btr";
#define MIN_X 0
#define MAX_X 255
#pragma pack(1)
typedef struct {
    uint8_t x;
    uint16_t xSquared;
    double xSquareRoot;
} record_t;
#pragma pack()
typedef uint8_t _key_t;
static Btrieve::StatusCode
createFile(BtrieveClient* btrieveClient)
{
    Btrieve::StatusCode status;
    BtrieveFileAttributes btrieveFileAttributes;
    // If SetFixedRecordLength() fails.
    if ((status = btrieveFileAttributes.SetFixedRecordLength(sizeof(record_t))) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveFileAttributes::SetFixedRecordLength():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
    // If FileCreate() fails.
    if ((status = btrieveClient->FileCreate(&btrieveFileAttributes, btrieveFileName, Btrieve::CREATE_MODE_OVERWRITE)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileCreate():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
        goto leave;
    }
leave:
    return status;
}
static Btrieve::StatusCode
openFile(BtrieveClient* btrieveClient, BtrieveFile* btrieveFile)
{
    Btrieve::StatusCode status;
    // If FileOpen() fails.
    if ((status = btrieveClient->FileOpen(btrieveFile, btrieveFileName, NULL, Btrieve::OPEN_MODE_NORMAL)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        printf("Error: BtrieveClient::FileOpen():%d:%s.\n", status, Btrieve::StatusCodeToString(status));
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

int main(int argc, char* argv[], char* envp[])
{

    if (argc < 2) {
        cout << "Must provide one argument." << endl;
        return -1;
    }
    if (argc >= 2) {
        cout << "Using first argument as file data." << endl;
    }

    BtrieveClient btrieveClient(0x4232, 0);
    Btrieve::StatusCode status = Btrieve::STATUS_CODE_UNKNOWN;
    BtrieveFile btrieveFile;
    _key_t key;
    uint64_t integerValue;
    // If the incorrect number of arguments were given.
    if (argc != 2)
    {
        printf("Usage: %s uint8_value\n", argv[0]);
        goto leave;
    }
    integerValue = atoi(argv[1]);
    // If integerValue is out of range.
    if ((integerValue < MIN_X) || (integerValue > MAX_X))
    {
        printf("Usage: %s uint8_value\n", argv[0]);
        goto leave;
    }
    key = (_key_t)integerValue;
    // If createFile() fails.
    if ((status = createFile(&btrieveClient)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If openFile() fails.
    if ((status = openFile(&btrieveClient, &btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If loadFile() fails.
    if ((status = loadFile(&btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If createIndex() fails.
    if ((status = createIndex(&btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If retrieveRecord() fails.
    if ((status = retrieveRecord(&btrieveFile, &key)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If closeFile() fails.
    if ((status = closeFile(&btrieveClient, &btrieveFile)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
    // If deleteFile() fails.
    if ((status = deleteFile(&btrieveClient)) != Btrieve::STATUS_CODE_NO_ERROR)
    {
        goto leave;
    }
leave:
    // If there wasn't a failure.
    if (status == Btrieve::STATUS_CODE_NO_ERROR)
        return 0;
    return 1;
}
