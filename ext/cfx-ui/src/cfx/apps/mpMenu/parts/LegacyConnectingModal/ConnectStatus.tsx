import { $L } from "cfx/common/services/intl/l10n";
import { Button } from "cfx/ui/Button/Button";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Modal } from "cfx/ui/Modal/Modal";
import { Text } from "cfx/ui/Text/Text";
import { noop } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";
import { ConnectState } from "../../services/servers/connect/state";

type ConnectStatusProps = {
  state: ConnectState.Status,

  onCancel?(): void,
}
export const ConnectStatus = observer(function ConnectStatus(props: ConnectStatusProps) {
  const {
    state,

    onCancel = noop,
  } = props;

  return (
    <>
      <Pad size="large">
        <Flex vertical gap="large">
          <Text size="xlarge">
            {$L('#Servers_Connecting')}
          </Text>

          {$L('#Servers_Message', state)}
        </Flex>
      </Pad>

      <Modal.Footer>
        <Button
          text={$L('#Servers_CancelOverlay')}
          disabled={!state.cancelable}
          onClick={onCancel}
        />
      </Modal.Footer>
    </>
  );
});
