#ifndef __BUILD_NUMBER_H_
#define __BUILD_NUMBER_H_

#define SVN_REVISION_STRING	"Unversioned directory"
#define SVN_REVISION	741
#define BUILD_MARK		120
#define VERSION_HASH	"cf8632557d4d518e30ad62f630ec5b56e2e3e963"

#define BUILD_TIME	"28/10/2022 15:25:57"
#define BUILD_NAME	"Tom Monkhouse"

#define BUILDNUMBER		(((SVN_REVISION & 0x00FF) << 8) | (BUILD_MARK & 0xFF))

#endif
