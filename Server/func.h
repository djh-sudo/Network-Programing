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
#define NO_INFORMATION "####"

#define MESSAGE_HEAD_DELETE "del##"
#define MESSAGE_HEAD_OTHER "oth##"
#define MESSAGE_HEAD_AGREEMENT_TO_BE_FRIEND "agr##"
#define MESSAGE_HEAD_TO_BE_FRIEND "uagr##"
#define MESSAGE_HEAD_SEARCH_FRIEND_OK "search##ok##"
#define MESSAGE_HEAD_SEARCH_FRIEND_OKK "search##okk##"
#define MESSAGE_HEAD_SEARCH_FRIEND_FAILED "search##fail##"
#define MESSAGE_HEAD_INITIAL "ini##"
#define MESSAGE_HEAD_LOGIN_FAILED "login##fail##"
#define MESSAGE_HEAD_REGISTER_OK "regis##ok##"
#define MESSAGE_HEAD_REGISTER_FAILED "regis##no##"
#define MESSAGE_HEAD_CREATE_GROUP_OK "crp##ok##"
#define MESSAGE_HEAD_CREATE_GROUP_FAILED "crp##no##"



__int64 Filetime2Int64(const FILETIME &ftime);
__int64 CompareFileTime2(const FILETIME &preTime, const FILETIME &nowTime);
float getCpuUsage();
#endif // FUNC_H
