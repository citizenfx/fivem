#include <DlcListState.h>

#include <CfxState.h>
#include <fmt/printf.h>
#include <Utils.h>

#include <cassert>
#include <optional>
#include <set>
#include <string>
#include <sstream>

namespace fx
{
    namespace client
    {

        const wchar_t* DlcManager::DCL_ALLOW_LIST_FLAG = L"-dlcAllowList";
        const wchar_t* DlcManager::DCL_BLOCK_LIST_FLAG = L"-dlcBlockList";

        std::set<std::string> DlcManager::_ParseDlcSet(const std::string& dlcs)
        {
            std::set<std::string> dlcSet;
            std::istringstream ss(dlcs);
            std::string dlcName;
            while (std::getline(ss, dlcName, ' '))
            {
                dlcSet.insert(std::move(dlcName));
            }

            return dlcSet;
        }

        const std::set<std::string>& DlcManager::_GetDlcSet()
        {
            static std::optional<std::set<std::string>> dlcSet;

            if (dlcSet.has_value())
            {
                return dlcSet.value();
            }

            auto sharedData = CfxState::Get();
            std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();

            size_t dlcListCommandIndex = cli.find(DCL_ALLOW_LIST_FLAG);
            if (dlcListCommandIndex == std::wstring_view::npos)
            {
                dlcListCommandIndex = cli.find(DCL_BLOCK_LIST_FLAG);
            }

            if (dlcListCommandIndex == std::wstring_view::npos)
            {
                dlcSet = std::set<std::string>();
                return dlcSet.value();
            }

            size_t dlcListStartIndex = cli.find_first_of(L"\"", dlcListCommandIndex);
            size_t dlcListEndIndex = cli.find_first_of(L"\"", dlcListStartIndex + 1);
            assert(dlcListStartIndex != std::wstring_view::npos && dlcListEndIndex != std::wstring_view::npos);

            std::string dlcList = ToNarrow(cli.substr(dlcListStartIndex + 1, dlcListEndIndex - dlcListStartIndex - 1));
            dlcSet = _ParseDlcSet(dlcList);
            return dlcSet.value();
        }

        bool DlcManager::_GetIsAllowList()
        {
            static std::optional<bool> isAllowList;

            if (isAllowList.has_value())
            {
                return isAllowList.value();
            }

            auto sharedData = CfxState::Get();
            std::wstring_view cli = (sharedData->initCommandLine[0]) ? sharedData->initCommandLine : GetCommandLineW();
            isAllowList = cli.find(DCL_ALLOW_LIST_FLAG) != std::wstring_view::npos;
            return isAllowList.value();
        }

        bool DlcManager::DoesDlcListDiffer(std::string newList, bool isAllowList)
        {
            return (isAllowList != _GetIsAllowList()) || (_ParseDlcSet(newList) != _GetDlcSet());
        }

        std::wstring DlcManager::CreateDlcListCommand(std::string newList, bool isAllowList)
        {
            if (newList.empty() && !isAllowList)
            {
                return L"";
            }

            return fmt::sprintf(L"%s \"%s\"", isAllowList ? DCL_ALLOW_LIST_FLAG : DCL_BLOCK_LIST_FLAG, ToWide(newList));
        }

        bool DlcManager::IsDlcBlocked(const std::string& dlcName)
        {
            bool isInSet = _GetDlcSet().find(dlcName) != _GetDlcSet().end();
            if (_GetIsAllowList())
            {
                return !isInSet;
            }

            return isInSet;
        }

    }
}
