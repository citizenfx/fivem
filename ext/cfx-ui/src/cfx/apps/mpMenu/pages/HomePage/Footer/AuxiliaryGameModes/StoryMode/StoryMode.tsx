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

import s from './StoryMode.module.scss';

export function StoryMode() {
  const [modalOpen, openModal, closeModal] = useOpenFlag(false);

  return (
    <>
      <Button
        size="large"
        theme="default-blurred"
        icon={Icons.storymode}
        text={$L('#BottomNav_Story')}
        onClick={openModal}
      />

      {modalOpen && (
        <Modal onClose={closeModal} backdropClassName={s.backdrop}>
          <Modal.Header>Enter Story Mode</Modal.Header>

          <Pad size="xlarge">
            <Flex centered vertical gap="xlarge">
              <TextBlock typographic centered size="large">
                Play GTA V story mode in FiveM, with addons loaded,
                <br />
                {/* eslint-disable-next-line react/no-unescaped-entities */}
                FiveM's engine improvements and seamless integration.
              </TextBlock>

              <TextBlock typographic centered size="large">
                You can place saved games in
                <br />
                <br />
                <kbd className="util-text-selectable">%USERPROFILE%\Saved Games\CitizenFX\GTA5</kbd>
              </TextBlock>

              <Button
                size="large"
                text="Launch Story Mode"
                theme="primary"
                onClick={() => mpMenu.invokeNative('executeCommand', 'storymode')}
              />
            </Flex>
          </Pad>
        </Modal>
      )}
    </>
  );
}
