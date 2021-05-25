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
#define ADD_FRIEND_OFFLINE "msg"
#define AGREEMENT_TO_BE_FRIEND_OFFLINE "uagr"
#define CREATE_GROUP "crp"
#define SEARCH_GROUP "searchg"
#define ADD_GROUP_COMPONENT "addg"
#define OTHER_UDP_MESSAGE "oth"
#define GROUP_MESSAGE "grp"
#define DELETE_USERS "del"
#define EXIT_FROM_GROUP "delg"
#define ADMIN_EXIT_FROM_GROUP "dela"
#define NEW_COMPONENT_ADD_FOR_OLD_OFFLINE "addgf"
#define NEW_COMPONENT_ADD_FOR_NEW  "addaf"
#define USERGROUP_DATABASE_TABLE "userGroup"
#define USERINFO_DATABASE_TABLE "userInfo"
#define SEGMENTATION "##"

#define MESSAGE_HEAD_ ""


__int64 Filetime2Int64(const FILETIME &ftime);
__int64 CompareFileTime2(const FILETIME &preTime, const FILETIME &nowTime);
float getCpuUsage();
#endif // FUNC_H
