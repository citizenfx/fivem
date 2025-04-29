import { ListTypeTabs } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { ServersPage } from "cfx/common/pages/ServersPage/ServersPage";
import { useService } from "cfx/base/servicesContainer";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { InsideNavBar } from "../../parts/NavBar/InsideNavBar";
import { Navigate } from "react-router-dom";
import { ServerFiltersWithDirectConnect } from "./ServerFiltersWithDirectConnect/ServerFiltersWithDirectConnect";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import { clsx } from "cfx/utils/clsx";
import s from './ServersPage.module.scss';

export interface ServersPageProps {
  listType: ServersListType,
}
export const MpMenuServersPage = observer(function ({ listType }: ServersPageProps) {
  const ServersService = useService(IServersService);

  const serversList = ServersService.getList(listType);
  if (!serversList) {
    return (
      <Navigate to="/servers" />
    );
  }

  const config = serversList.getConfig?.();
  const showPinned = currentGameNameIs(GameName.FiveM) && listType === ServersListType.All;

  const navbarClassName = clsx(s.navbar, {
    [s.shrink]: showPinned,
  });

  return (
    <>
      <InsideNavBar>
        <Flex gap="large" className={navbarClassName}>
          <ListTypeTabs />

          {!!config && (
            <ServerFiltersWithDirectConnect config={config} />
          )}
        </Flex>
      </InsideNavBar>

      <ServersPage
        list={serversList}
        listType={listType}
        showPinned={showPinned}
      />
    </>
  );
});
