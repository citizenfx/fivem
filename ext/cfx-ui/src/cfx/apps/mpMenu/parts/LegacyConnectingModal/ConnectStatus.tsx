import {
  Button,
  Flex,
  Pad,
  Modal,
  Text,
  noop,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { $L, useL10n } from 'cfx/common/services/intl/l10n';
import { nl2brx } from 'cfx/utils/nl2br';

import { ConnectState } from '../../services/servers/connect/state';

type ConnectStatusProps = {
  state: ConnectState.Status;

  onCancel?(): void;
};
export const ConnectStatus = observer(function ConnectStatus(props: ConnectStatusProps) {
  const {
    state,

    onCancel = noop,
  } = props;

  const message = useL10n('#Servers_Message', state);

  return (
    <>
      <Pad size="large">
        <Flex vertical gap="large">
          <Text size="xlarge">{$L('#Servers_Connecting')}</Text>

          {nl2brx(message)}
        </Flex>
      </Pad>

      <Modal.Footer>
        <Button text={$L('#Servers_CancelOverlay')} disabled={!state.cancelable} onClick={onCancel} />
      </Modal.Footer>
    </>
  );
});
