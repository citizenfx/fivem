#pragma once

#define ROS_SUFFIX "_b576"
#define ROS_SUFFIX_W L"_b576"

//Social Club DLL versions
#ifndef GTA_NY
#define SOCIAL_CLUB_VERSION {2, 0, 9, 0}
#define SOCIAL_CLUB_CEF_VERSION {85, 3, 9, 0}
#else
//LibertyM uses an older version of Social Club
#define SOCIAL_CLUB_VERSION {2, 0, 7, 9}
#define SOCIAL_CLUB_CEF_VERSION {83, 5, 0, 0}
#endif
