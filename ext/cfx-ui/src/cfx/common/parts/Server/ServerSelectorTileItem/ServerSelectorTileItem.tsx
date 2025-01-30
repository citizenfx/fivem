import {
  Icon,
  Icons,
  Interactive,
  Box,
  Flex,
  FlexRestricter,
  Pad,
  Modal,
  PremiumBadge,
  Text,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { useServersService } from 'cfx/common/services/servers/servers.service';
import { IPinnedServersCollection, IServerView } from 'cfx/common/services/servers/types';
import { useOpenFlag } from 'cfx/utils/hooks';

import { ServerIcon } from '../ServerIcon/ServerIcon';
import { ServerPower } from '../ServerPower/ServerPower';
import { ServerTileItem, ServerTileItemProps } from '../ServerTileItem/ServerTileItem';
import { ServerTitle } from '../ServerTitle/ServerTitle';

import s from './ServerSelectorTileItem.module.scss';

export type ServerSelectorTileItemProps = Omit<ServerTileItemProps, 'server'> & {
  serversCollection: IPinnedServersCollection;
};

export const ServerSelectorTileItem = observer(function ServerSelectorTileItem(props: ServerSelectorTileItemProps) {
  const {
    label,
    serversCollection,
  } = props;

  const ServersService = useServersService();

  const [modalOpen, openModal, closeModal] = useOpenFlag(false);

  let totalPlayers = 0;

  const servers = serversCollection.ids.map((serverId) => {
    const server = ServersService.getServer(serverId);

    if (server) {
      totalPlayers += server.playersCurrent || 0;
    }

    return server;
  });

  const firstServer = servers[0];

  if (!firstServer) {
    return null;
  }

  return (
    <>
      <Interactive className={s.root} onClick={openModal}>
        <Flex vertical fullHeight>
          <Flex repell>
            {label}

            <Flex centered="axis" gap="small">
              <Icon opacity="50">{Icons.playersCount}</Icon>
              <Text opacity="75">{totalPlayers}</Text>
            </Flex>
          </Flex>

          <Flex fullWidth>
            <Box height={10}>
              <ServerIcon type="list" server={firstServer} />
            </Box>

            <FlexRestricter>
              <Flex vertical fullHeight fullWidth centered="cross-axis" gap="small">
                <ServerTitle truncated size="xlarge" title={serversCollection.title} />

                {!!firstServer.projectDescription && (
                  <Title delay={500} fixedOn="bottom-left" title={firstServer.projectDescription}>
                    <Text truncated opacity="50">
                      {firstServer.projectDescription}
                    </Text>
                  </Title>
                )}
              </Flex>
            </FlexRestricter>
          </Flex>
        </Flex>
      </Interactive>

      {modalOpen && (
        <ServerSelectorModal
          title={serversCollection.title}
          servers={servers}
          onClose={closeModal}
          totalPlayers={totalPlayers}
        />
      )}
    </>
  );
});

interface ServerSelectorModalProps {
  title: string;
  servers: Array<IServerView | undefined>;
  totalPlayers: number;
  onClose(): void;
}

const ServerSelectorModal = observer(function ServerSelectorModal(props: ServerSelectorModalProps) {
  const {
    title,
    servers,
    totalPlayers,
    onClose,
  } = props;

  const firstServer = servers[0];

  if (!firstServer) {
    return null;
  }

  const serverNodes = servers.map((server) => {
    if (!server) {
      return null;
    }

    return (
      <ServerTileItem
        key={server.id}
        hideBanner
        hideBoost
        hideIcon
        placeControlsBelow
        hideDescription
        server={server}
        elementPlacement={ElementPlacements.ServerSelectorModal}
      />
    );
  });

  return (
    <Modal onClose={onClose}>
      <Flex vertical gap="none">
        <Pad size="xlarge">
          <Flex vertical centered>
            <div />
            <div />
            <ServerIcon server={firstServer} type="details" />

            <div />

            <ServerTitle size="xxxlarge" title={title} />

            <Flex centered="axis" gap="normal">
              <Flex centered="axis" gap="thin">
                <Icon opacity="75">{Icons.playersCount}</Icon>
                <Text>{totalPlayers}</Text>
              </Flex>

              <ServerPower server={firstServer} />

              <PremiumBadge level={firstServer.premium!} />
            </Flex>

            <Box width="75%">
              <Flex centered>
                <Text centered typographic opacity="50">
                  {firstServer.projectDescription}
                </Text>
              </Flex>
            </Box>
          </Flex>
        </Pad>

        <Pad size="large">
          <Flex vertical>{serverNodes}</Flex>
        </Pad>
      </Flex>
    </Modal>
  );
});
