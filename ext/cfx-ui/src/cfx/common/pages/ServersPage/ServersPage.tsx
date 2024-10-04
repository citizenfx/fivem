import {
  Icons,
  Island,
  Box,
  Flex,
  FlexRestricter,
  Pad,
  Page,
  Scrollable,
  VirtualScrollable,
  Text,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { IndexedServerListItem } from 'cfx/common/parts/Server/ServerListItem/IndexedServerListItem';
import { ServerListItem } from 'cfx/common/parts/Server/ServerListItem/ServerListItem';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { IServersList, ServersListType } from 'cfx/common/services/servers/lists/types';
import { useServersService } from 'cfx/common/services/servers/servers.service';
import { useUiService } from 'cfx/common/services/ui/ui.service';
import { useSavedScrollPositionForBackNav } from 'cfx/utils/hooks';

import { EmptyListPlaceholder } from './EmptyListPlaceholder/EmptyListPlaceholder';

import s from './ServersPage.module.scss';

const emptyListPlaceholders = {
  [ServersListType.History]: true,
  [ServersListType.Favorites]: true,
};

export interface ServersPageProps {
  list: IServersList;
  listType?: ServersListType;
  showPinned?: boolean;
}

export const ServersPage = observer(function ServersPage(props: ServersPageProps) {
  const {
    list,
    listType,
    showPinned = false,
  } = props;

  const UiService = useUiService();
  const ServersService = useServersService();

  const [initialScrollOffset, setScrollOffset] = useSavedScrollPositionForBackNav(list);

  const renderItem = React.useCallback((index: number) => (
    <IndexedServerListItem index={index} list={list} />
  ), [list]);

  const isListEmpty = list.sequence.length === 0;
  const isListLoading = ServersService.serversListLoading;

  const hasEmptyListPlaceholder = listType && emptyListPlaceholders[listType];

  const showPlaceholder = Boolean(hasEmptyListPlaceholder && isListEmpty && !isListLoading);

  return (
    <Page showLoader={ServersService.serversListLoading}>
      <Flex fullHeight fullWidth>
        {showPlaceholder && (
          <EmptyListPlaceholder configController={list.getConfig?.()} />
        )}

        {!showPlaceholder && (
          <VirtualScrollable
            onScrollUpdate={setScrollOffset}
            initialScrollOffset={initialScrollOffset}
            className={s.list}
            itemCount={list.sequence.length}
            itemHeight={UiService.quant * 8}
            renderItem={renderItem}
          />
        )}

        {showPinned && (
          <PinnedServers />
        )}
      </Flex>
    </Page>
  );
});

const PinnedServers = observer(function PinnedServers() {
  const ServersService = useServersService();

  const nodes = ServersService.pinnedServers
    .map((id) => ServersService.getServer(id))
    .filter(Boolean)
    .map((server) => (
      <Box key={server!.id} height={10} width="100%">
        <ServerListItem
          standalone
          hideTags
          hideActions
          hideCountryFlag
          hidePremiumBadge
          descriptionUnderName
          server={server}
          elementPlacement={ElementPlacements.ServerFeaturedList}
        />
      </Box>
    ));

  return (
    <Island className={s.pinned}>
      <Flex vertical fullHeight gap="none">
        <Pad>
          <Flex centered="axis">
            <Text size="large" opacity="50">
              {Icons.serversFeatured}
            </Text>
            <Text size="large" weight="bold" opacity="50">
              {$L('#ServerList_FeaturedServers')}
            </Text>
          </Flex>
        </Pad>

        <FlexRestricter vertical>
          <Scrollable>
            <Pad left right bottom>
              <Flex vertical gap="small">
                {nodes}
              </Flex>
            </Pad>
          </Scrollable>
        </FlexRestricter>
      </Flex>
    </Island>
  );
});
