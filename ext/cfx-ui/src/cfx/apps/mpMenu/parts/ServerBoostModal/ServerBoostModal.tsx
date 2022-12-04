import React from "react";
import { useService } from "cfx/base/servicesContainer";
import { Button } from "cfx/ui/Button/Button";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Modal } from "cfx/ui/Modal/Modal";
import { observer } from "mobx-react-lite";
import { BoostUIState, MpMenuServersBoostService } from "../../services/servers/serversBoost.mpMenu";
import { TextBlock } from "cfx/ui/Text/Text";
import { Flex } from "cfx/ui/Layout/Flex/Flex";

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
          Your BOOST™ is now assigned to this server (with an admirable strength of {ServersBoostService.currentBoost?.power})!
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
      <Modal.Header>
        Applying server BOOST™
      </Modal.Header>

      <Pad top bottom size="large">
        <Pad>
          <TextBlock size="xlarge">
            {node}
          </TextBlock>
        </Pad>
      </Pad>

      {!isBoosting && (
        <Modal.Footer>
          <Button
            text="Close"
            onClick={ServersBoostService.closeUi}
          />
        </Modal.Footer>
      )}
    </Modal>
  );
});
