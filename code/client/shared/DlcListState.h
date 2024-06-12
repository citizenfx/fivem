#pragma once

#include <StdInc.h>

#include <optional>
#include <set>
#include <string>
#include <sstream>

namespace fx
{
    namespace client
    {

        class DlcManager
        {
        private:
            static const wchar_t* DCL_ALLOW_LIST_FLAG;
            static const wchar_t* DCL_BLOCK_LIST_FLAG;

            static std::set<std::string> _ParseDlcSet(const std::string& dlcs);

            static const std::set<std::string>& _GetDlcSet();

            static bool _GetIsAllowList();

        public:

            static bool DoesDlcListDiffer(std::string newList, bool isAllowList);

            static std::wstring CreateDlcListCommand(std::string newList, bool isAllowList);

            static bool IsDlcBlocked(const std::string& dlcName);
        };
    }
}
