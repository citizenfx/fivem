import React from 'react';

import { CurrentGameBrand } from 'cfx/base/gameRuntime';
import { $L, useL10n } from 'cfx/common/services/intl/l10n';
import { Button } from 'cfx/ui/Button/Button';
import { Flex } from 'cfx/ui/Layout/Flex/Flex';
import { Pad } from 'cfx/ui/Layout/Pad/Pad';
import { Modal } from 'cfx/ui/Modal/Modal';
import { Text } from 'cfx/ui/Text/Text';
import { noop } from 'cfx/utils/functional';
import { nl2brx } from 'cfx/utils/nl2br';

import { mpMenu } from '../../mpMenu';
import { ConnectState } from '../../services/servers/connect/state';

export interface BuildSwitchRequestProps {
  state: ConnectState.BuildSwitchRequest;
  onCancel?(): void;
}
export function BuildSwitchRequest(props: BuildSwitchRequestProps) {
  const {
    state,
    onCancel = noop,
  } = props;

  const [secondsLeft, setSecondsLeft] = React.useState(10);

  const performBuildSwitch = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'ok' });
  }, []);
  const cancelBuildSwitch = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'cancel' });
    onCancel();
  }, [onCancel]);

  React.useEffect(() => {
    const timerHandler = () => {
      setSecondsLeft((left) => {
        const newLeft = left - 1;

        if (newLeft <= 0) {
          clearInterval(timer);

          performBuildSwitch();
        }

        return newLeft;
      });
    };

    const timer = setInterval(timerHandler, 1000);

    return () => clearInterval(timer);
  }, []);

  const textData = React.useMemo(
    () => ({
      ...state,
      gameBrand: CurrentGameBrand,
    }),
    [state],
  );

  const buildSwitchBodyLocalized = useL10n(getBuildSwitchBody(state), textData);

  return (
    <>
      <Pad>
        <Flex vertical>
          <Text size="xlarge">{$L('#BuildSwitch_Heading', textData)}</Text>

          <Text size="large">{nl2brx(buildSwitchBodyLocalized)}</Text>
        </Flex>
      </Pad>

      <Modal.Footer>
        <Flex>
          <Button text={$L('#BuildSwitch_OK', { seconds: secondsLeft })} theme="primary" onClick={performBuildSwitch} />

          <Button text={$L('#BuildSwitch_Cancel', { seconds: secondsLeft })} onClick={cancelBuildSwitch} />
        </Flex>
      </Modal.Footer>
    </>
  );
}

function getBuildSwitchBody(switchRequest: ConnectState.BuildSwitchRequest) {
  const buildChanged = switchRequest.currentBuild !== switchRequest.build;
  const pureLevelChanged = switchRequest.currentPureLevel !== switchRequest.pureLevel;

  if (pureLevelChanged) {
    if (switchRequest.currentPureLevel === 0) {
      if (!buildChanged) {
        return '#BuildSwitch_PureBody';
      }

      return '#BuildSwitch_PureBuildBody';
    }

    if (!buildChanged) {
      return '#BuildSwitch_PureSwitchBody';
    }

    return '#BuildSwitch_PureBuildSwitchBody';
  }

  return '#BuildSwitch_Body';
}
