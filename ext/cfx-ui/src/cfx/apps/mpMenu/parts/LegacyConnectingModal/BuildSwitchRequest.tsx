import {
  Button,
  Flex,
  Pad,
  Modal,
  Text,
  noop,
} from '@cfx-dev/ui-components';
import React from 'react';

import { CurrentGameBrand } from 'cfx/base/gameRuntime';
import { $L, useL10n } from 'cfx/common/services/intl/l10n';
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

  const [secondsLeft, setSecondsLeft] = React.useState(state.optional ? 5 : 10);

  const performBuildSwitch = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'ok' });
  }, []);
  const skipBuildSwitch = React.useCallback(() => {
    mpMenu.submitAdaptiveCardResponse({ action: 'skip' });
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

          (state.optional ? skipBuildSwitch : performBuildSwitch)();
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
      sizeIncreasePerPool: formatPoolSizesIncrease(state.poolSizesIncrease),
      gameBrand: CurrentGameBrand,
    }),
    [state],
  );

  return (
    <>
      <Pad>
        <Flex vertical>
          <Text size="xlarge">{$L('#BuildSwitch_Heading', textData)}</Text>

          {state.optional === true
            && <Text size="large">{$L('#BuildSwitch_OptionalChange')}</Text>
            || <Text size="large">{$L('#BuildSwitch_GeneralDescription', textData)}</Text>
          }
          {state.currentBuild !== state.build && <Text size="large">{$L('#BuildSwitch_BuildDiff', textData)}</Text>}
          {state.currentPureLevel !== state.pureLevel && <Text size="large">{$L('#BuildSwitch_PureModeDiff', textData)}</Text>}
          {state.currentReplaceExecutable !== state.replaceExecutable && <Text size="large">{
            state.replaceExecutable === true ? $L('#BuildSwitch_ReplaceExecutable', textData) : $L('#BuildSwitch_DontReplaceExecutable', textData)
          }</Text>}
          {state.currentPoolSizesIncrease !== state.poolSizesIncrease && <Text size="large">{
            state.poolSizesIncrease === "" ? $L('#BuildSwitch_PoolSizeDefault', textData) : $L('#BuildSwitch_PoolSizeDiff', textData)
          }</Text>}
        </Flex>
      </Pad>

      <Modal.Footer>
        <Flex>
          {state.optional && (
            <>
              <Button text={$L('#BuildSwitch_Skip', { seconds: secondsLeft })} theme="primary" onClick={skipBuildSwitch} />

              <Button text={$L('#BuildSwitch_Continue')} onClick={performBuildSwitch} />
            </>
          ) || (
            <>
              <Button text={$L('#BuildSwitch_OK', { seconds: secondsLeft })} theme="primary" onClick={performBuildSwitch} />

              <Button text={$L('#BuildSwitch_Cancel', { seconds: secondsLeft })} onClick={cancelBuildSwitch} />
            </>
          )}
        </Flex>
      </Modal.Footer>
    </>
  );
}

function formatPoolSizesIncrease(request: string) {
  return request.replace(/[\{\}\"]/g, '').replace(/:/g, ': ').replace(/,/g, ', ');
}
