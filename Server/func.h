#ifndef FUNC_H
#define FUNC_H

#include<windows.h>

/*********************  Macro defination *********************/
#define KB 1024
#define MB 1024*1024
#define GB (MB*KB)

#define USER_LOGIN "login"
#define USER_REGISTER "regis"
#define USER_ADD_FRIEND "add"
#define USER_SEARCH "search"
#define USER_INICIAL "ini"
#define AGREEMENT_TO_BE_FRIEND "agr"
#define CREATE_GROUP "crp"
#define SEARCH_GROUP "searchg"
#define ADD_GROUP_COMPONENT "addg"
#define OTHER_UDP_MESSAGE "oth"
#define GROUP_MESSAGE "grp"
#define DELETE_USERS "del"

__int64 Filetime2Int64(const FILETIME &ftime);
__int64 CompareFileTime2(const FILETIME &preTime, const FILETIME &nowTime);
float getCpuUsage();
#endif // FUNC_H
