import { observer } from 'mobx-react-lite';

import { useService } from 'cfx/base/servicesContainer';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { IServersList } from 'cfx/common/services/servers/lists/types';
import { IServersService } from 'cfx/common/services/servers/servers.service';

import { ServerListItem, ServerListItemProps } from './ServerListItem';

export type IndexedServerListItemProps = Omit<ServerListItemProps, 'server' | 'pinned'> & {
  list: IServersList;
  index: number;

  ignorePinned?: boolean;
};

export const IndexedServerListItem = observer(function IndexedServerListItem(props: IndexedServerListItemProps) {
  const {
    list,
    index,
    ignorePinned = false,
    ...rest
  } = props;

  const ServersService = useService(IServersService);

  const serverId = list.sequence[index];
  const server = ServersService.getServer(serverId);

  const pinned = ignorePinned
    ? false
    : ServersService.isServerPinned(serverId);

  return (
    <ServerListItem {...rest} pinned={pinned} server={server} elementPlacement={ElementPlacements.ServerList} />
  );
});
