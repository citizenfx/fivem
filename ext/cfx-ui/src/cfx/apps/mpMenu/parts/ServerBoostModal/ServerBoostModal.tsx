import {
  Button,
  Indicator,
  Flex,
  Pad,
  Modal,
  TextBlock,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useService } from 'cfx/base/servicesContainer';

import { BoostUIState, MpMenuServersBoostService } from '../../services/servers/serversBoost.mpMenu';

export const ServerBoostModal = observer(function ServerBoostModal() {
  const ServersBoostService = useService(MpMenuServersBoostService);

  if (ServersBoostService.uiState === BoostUIState.Idle) {
    return null;
  }

  const isBoosting = ServersBoostService.uiState === BoostUIState.Boosting;

  let node: React.ReactNode;

  switch (ServersBoostService.uiState) {
    case BoostUIState.Boosting: {
      node = (
        <Flex centered>
          <Indicator />
        </Flex>
      );
      break;
    }

    case BoostUIState.NoAccount: {
      node = 'You need to have a linked FiveM account in order to BOOST™ a server.';
      break;
    }

    case BoostUIState.Success: {
      node = (
        <>
          Your BOOST™ is now assigned to this server (with an admirable strength of{' '}
          {ServersBoostService.currentBoost?.power})!
          <br />
          <br />
          Thanks for helping the server go higher.
        </>
      );
      break;
    }

    case BoostUIState.Error: {
      node = ServersBoostService.uiError;
      break;
    }
  }

  const handleClose = isBoosting
    ? undefined
    : ServersBoostService.closeUi;

  return (
    <Modal onClose={handleClose}>
      <Modal.Header>Applying server BOOST™</Modal.Header>

      <Pad top bottom size="large">
        <Pad>
          <TextBlock size="xlarge">{node}</TextBlock>
        </Pad>
      </Pad>

      {!isBoosting && (
        <Modal.Footer>
          <Button text="Close" onClick={ServersBoostService.closeUi} />
        </Modal.Footer>
      )}
    </Modal>
  );
});
