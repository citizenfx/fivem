import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { $L } from "cfx/common/services/intl/l10n";
import { Button, ButtonProps } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Modal } from "cfx/ui/Modal/Modal";
import { TextBlock } from "cfx/ui/Text/Text";
import { useOpenFlag } from "cfx/utils/hooks";
import s from './ReplayEditor.module.scss';

export function ReplayEditor() {
  const [modalOpen, openModal, closeModal] = useOpenFlag(false);

  return (
    <>
      <Button
        fullWidth
        size="large"
        theme="default-blurred"
        icon={Icons.replayEditor}
        text={$L('#BottomNav_ReplayEditor')}
        onClick={openModal}
      />

      {modalOpen && (
        <Modal onClose={closeModal} backdropClassName={s.backdrop}>
          <Modal.Header>
            Enter the Rockstar Editor
          </Modal.Header>

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
