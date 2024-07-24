import {
  Button,
  Flex,
  Pad,
  Modal,
  Text,
  TextBlock,
  noop,
} from '@cfx-dev/ui-components';
import React from 'react';

import { $L } from 'cfx/common/services/intl/l10n';

import { mpMenu } from '../../mpMenu';
import { ConnectState } from '../../services/servers/connect/state';

export interface BuildSwitchInfoProps {
  state: ConnectState.BuildSwitchInfo;
  onCancel?(): void;
}
export function BuildSwitchInfo(props: BuildSwitchInfoProps) {
  const {
    state,
    onCancel = noop,
  } = props;

  const handleOk = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'ok' });
  }, []);
  const handleCancel = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'cancel' });
    onCancel();
  }, [onCancel]);

  return (
    <>
      <Pad>
        <Flex vertical>
          <Text size="xlarge">{state.title}</Text>

          <TextBlock size="large">{state.content}</TextBlock>
        </Flex>
      </Pad>

      <Modal.Footer>
        <Flex>
          <Button text={$L('#Yes')} theme="primary" onClick={handleOk} />

          <Button text={$L('#No')} onClick={handleCancel} />
        </Flex>
      </Modal.Footer>
    </>
  );
}
