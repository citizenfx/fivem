import {
  Indicator,
  Box,
  Flex,
  Pad,
  Modal,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { $L } from 'cfx/common/services/intl/l10n';

import { AdaptiveCardPresenter } from './AdaptiveCardPresenter/AdaptiveCardPresenter';
import { BuildSwitchInfo } from './BuildSwitchInfo';
import { BuildSwitchRequest } from './BuildSwitchRequest';
import { ConnectFailed } from './ConnectFailed';
import { ConnectStatus } from './ConnectStatus';
import { ServerHeader } from './ServerHeader';
import { useMpMenuServersConnectService } from '../../services/servers/serversConnect.mpMenu';

export const LegacyConnectingModal = observer(function LegacyConnectingModal() {
  const service = useMpMenuServersConnectService();

  if (!service.showModal) {
    return null;
  }

  let node: React.ReactNode;

  if (service.resolvingServer) {
    node = (
      <ResolvingServer />
    );
  } else if (service.state) {
    switch (service.state.type) {
      case 'connecting': {
        node = (
          <ResolvingServer />
        );
        break;
      }

      case 'status': {
        node = (
          <ConnectStatus state={service.state} onCancel={service.cancel} />
        );
        break;
      }

      case 'failed': {
        node = (
          <ConnectFailed state={service.state} server={service.server!} onClose={service.cancel} />
        );
        break;
      }

      case 'card': {
        node = (
          <AdaptiveCardPresenter card={service.state.card} onCancel={service.cancel} />
        );
        break;
      }

      case 'buildSwitchRequest': {
        node = (
          <BuildSwitchRequest state={service.state} onCancel={service.cancel} />
        );
        break;
      }
      case 'buildSwitchInfo': {
        node = (
          <BuildSwitchInfo state={service.state} onCancel={service.cancel} />
        );
        break;
      }
    }
  }

  return (
    <Modal
      disableBackdropClose
      onClose={service.canCancel
        ? service.cancel
        : undefined}
    >
      <Box width="calc(var(--width) / 2)">
        {!!service.server && service.showServer && (
          <ServerHeader server={service.server} />
        )}

        {node}
      </Box>
    </Modal>
  );
});

function ResolvingServer() {
  return (
    <Pad size="xlarge">
      <Flex centered>
        <Indicator />

        {$L('#Servers_ConnectingToServer')}
      </Flex>
    </Pad>
  );
}
