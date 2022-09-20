import React from "react";
import { observer } from "mobx-react-lite";
import { Page } from "cfx/ui/Layout/Page/Page";
import { ServerListItem } from "cfx/common/parts/Server/ServerListItem/ServerListItem";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { IServersList } from "cfx/common/services/servers/lists/types";
import { VirtualScrollable } from "cfx/ui/Layout/Scrollable/VirtualScrollable";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { Island } from "cfx/ui/Island/Island";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { Text } from "cfx/ui/Text/Text";
import { GiPieSlice } from 'react-icons/gi';
import { useUiService } from "cfx/common/services/ui/ui.service";
import { useSavedScrollPositionForBackNav } from "cfx/utils/hooks";
import s from './ServersPage.module.scss';
import { $L } from "cfx/common/services/intl/l10n";
import { Icons } from "cfx/ui/Icons";

export interface ServersPageProps {
  list: IServersList,
  showPinned?: boolean,
}

export const ServersPage = observer(function ServersPage(props: ServersPageProps) {
  const {
    list,
    showPinned = false,
  } = props;

  const UiService = useUiService();
  const ServersService = useServersService();

  const [initialScrollOffset, setScrollOffset] = useSavedScrollPositionForBackNav(list);

  const renderItem = React.useCallback((index: number) => (
    <ServerListItem
      pinned={ServersService.isServerPinned(list.sequence[index])}
      server={ServersService.getServer(list.sequence[index])}
    />
  ), [list, ServersService]);

  return (
    <Page showLoader={ServersService.serversListLoading}>
      <Flex fullHeight fullWidth>
        <VirtualScrollable
          onScrollUpdate={setScrollOffset}
          initialScrollOffset={initialScrollOffset}

          className={s.list}
          itemCount={list.sequence.length}
          itemHeight={UiService.quant * 8}
          renderItem={renderItem}
        />

        {showPinned && (
          <PinnedServers />
        )}
      </Flex>
    </Page>
  );
});

const PinnedServers = observer(function PinnedServers() {
  const ServersService = useServersService();

  const nodes = ServersService.pinnedServers.map((serverId) => (
    <Box
      key={serverId}
      height={10}
      width="100%"
    >
      <ServerListItem
        standalone
        hideTags
        hideActions
        hideCountryFlag
        hidePremiumBadge
        descriptionUnderName
        server={ServersService.getServer(serverId)}
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
