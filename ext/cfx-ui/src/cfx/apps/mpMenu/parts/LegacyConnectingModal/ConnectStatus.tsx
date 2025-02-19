import {
  Button,
  Flex,
  Pad,
  Modal,
  Text,
  noop,
  linkify
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { $L } from 'cfx/common/services/intl/l10n';
import { nl2br } from 'cfx/utils/nl2br';
import { html2react } from 'cfx/utils/html2react';

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

  const { message, cancelable } = state;

  return (
    <>
      <Pad size="large">
        <Flex vertical gap="large">
          <Text size="xlarge">{$L('#Servers_Connecting')}</Text>

          {html2react(linkify(nl2br(message)))}
        </Flex>
      </Pad>

      <Modal.Footer>
        <Button text={$L('#Servers_CancelOverlay')} disabled={!cancelable} onClick={onCancel} />
      </Modal.Footer>
    </>
  );
});
