#ifndef TCPS_H
#define TCPS_H

// Total Commander Plugins Specifications

#define lc_copy		1
#define lc_newparams	2
#define lc_selectall	3
#define lc_setpercent	4

#define lcp_wraptext	1
#define lcp_fittowindow 2
#define lcp_ansi		4
#define lcp_ascii		8
#define lcp_variable	12
#define lcp_forceshow	16

#define lcs_findfirst	1
#define lcs_matchcase	2
#define lcs_wholewords	4
#define lcs_backwards	8

#define itm_percent	0xFFFE
#define itm_fontstyle	0xFFFD
#define itm_wrap		0xFFFC
#define itm_fit		0xFFFB
#define itm_next	0xFFFA

#define LISTPLUGIN_OK	0
#define LISTPLUGIN_ERROR	1

#include <Windows.h>

typedef struct {
	int size;
	DWORD PluginInterfaceVersionLow;
	DWORD PluginInterfaceVersionHi;
	char DefaultIniName[MAX_PATH];
} ListDefaultParamStruct;

#endif // TCPS_H