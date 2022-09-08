import React from "react";
import { $L } from "cfx/common/services/intl/l10n";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Modal } from "cfx/ui/Modal/Modal";
import { observer } from "mobx-react-lite";
import { useMpMenuServersConnectService } from "../../services/servers/serversConnect.mpMenu";
import { BuildSwitchRequest } from "./BuildSwitchRequest";
import { ServerHeader } from "./ServerHeader";
import { ConnectStatus } from "./ConnectStatus";
import { AdaptiveCardPresenter } from "./AdaptiveCardPresenter/AdaptiveCardPresenter";
import { ConnectFailed } from "./ConnectFailed";
import { BuildSwitchInfo } from "./BuildSwitchInfo";
import { Box } from "cfx/ui/Layout/Box/Box";

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
          <ConnectStatus
            state={service.state}
            onCancel={service.cancel}
          />
        );
        break;
      }

      case 'failed': {
        node = (
          <ConnectFailed
            state={service.state}
            server={service.server!}
            onClose={service.cancel}
          />
        );
        break;
      }

      case 'card': {
        node = (
          <AdaptiveCardPresenter
            card={service.state.card}
            onCancel={service.cancel}
          />
        );
        break;
      }

      case 'buildSwitchRequest': {
        node = (
          <BuildSwitchRequest
            state={service.state}
            onCancel={service.cancel}
          />
        );
        break;
      }
      case 'buildSwitchInfo': {
        node = (
          <BuildSwitchInfo
            state={service.state}
            onCancel={service.cancel}
          />
        );
        break;
      }
    }
  }

  return (
    <Modal onClose={service.canCancel ? service.cancel : undefined}>
      <Box width="calc(var(--width) / 2)">
        {!!service.server && (
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

        {$L('Connecting to server...')}
      </Flex>
    </Pad>
  );
}
