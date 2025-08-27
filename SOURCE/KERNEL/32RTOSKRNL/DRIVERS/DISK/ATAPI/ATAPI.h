#ifndef ATAPI_DRIVER_H
#define ATAPI_DRIVER_H
#include "../ATA_SHARED.h"

#define ATAPI_SECTOR_SIZE 2048
#define ATAPI_SECTORS_U8(num) (ATAPI_SECTOR_SIZE * (num))

#define ATAPI_SECTORS(num) (ATAPI_SECTOR_SIZE / 2 * (num))

#define ATAPI_FAILED                0
#define ATAPI_SUCCESS               1
#define ATAPI_PRIMARY_MASTER        'A'//1
#define ATAPI_PRIMARY_SLAVE         'B'//2
#define ATAPI_SECONDARY_MASTER      'C'//3
#define ATAPI_SECONDARY_SLAVE       'D'//4
/*+++
U32 ATAPI_CHECK(U0);

DESCRIPTION
    Checks which ATAPI CD-ROM drives are present.
    Save the results as it will be used later if you need to access data from ATAPI
    
RETURN
    ATAPI_FAILED
    ATAPI_PRIMARY_MASTER
    ATAPI_PRIMARY_SLAVE
    ATAPI_SECONDARY_MASTER
    ATAPI_SECONDARY_SLAVE

AUTHORS
    Antonako1

REVISION HISTORY
    2025/08/27 - Antonako1
        Initial version.
---*/
U32 ATAPI_CHECK(U0);


BOOLEAN READ_CDROM(U32 ATAPICheckRes, U32 lba, U32 sectors, U16 *buf);

#endif // ATAPI_DRIVER_H