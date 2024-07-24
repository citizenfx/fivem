import {
  Button,
  Icons,
  Flex,
  Pad,
  Modal,
  TextBlock,
} from '@cfx-dev/ui-components';

import { mpMenu } from 'cfx/apps/mpMenu/mpMenu';
import { $L } from 'cfx/common/services/intl/l10n';
import { useOpenFlag } from 'cfx/utils/hooks';

import s from './ReplayEditor.module.scss';

export function ReplayEditor() {
  const [modalOpen, openModal, closeModal] = useOpenFlag(false);

  return (
    <>
      <Button
        size="large"
        theme="default-blurred"
        icon={Icons.replayEditor}
        text={$L('#BottomNav_ReplayEditor')}
        onClick={openModal}
      />

      {modalOpen && (
        <Modal onClose={closeModal} backdropClassName={s.backdrop}>
          <Modal.Header>Enter the Rockstar Editor</Modal.Header>

          <Pad size="xlarge">
            <Flex centered vertical gap="xlarge">
              <TextBlock typographic centered size="large">
                Open the Rockstar Editor to edit,
                <br />
                arrange and export saved clips created
                <br />
                in FiveM Story Mode, Multiplayer or elsewhere.
              </TextBlock>

              <Button
                size="large"
                text="Launch Replay Editor"
                theme="primary"
                onClick={() => mpMenu.invokeNative('executeCommand', 'replayEditor')}
              />

              <TextBlock typographic centered size="small" opacity="50">
                More editor features are planned for a later release.
              </TextBlock>
            </Flex>
          </Pad>
        </Modal>
      )}
    </>
  );
}
