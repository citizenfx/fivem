import {
  Button,
  ButtonBar,
  Flex,
  Pad,
  Modal,
  TextBlock,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { useService } from 'cfx/base/servicesContainer';
import { $L } from 'cfx/common/services/intl/l10n';

import { IUiMessage } from '../../services/uiMessage/types';
import { IUiMessageService } from '../../services/uiMessage/uiMessage.service';
import { useRenderedFormattedMessage } from '../../utils/messageFormatting';

export const LegacyUiMessageModal = observer(function LegacyUiMessageModal() {
  const UiMessageService = useService(IUiMessageService);

  const {
    message,
  } = UiMessageService;

  if (!message) {
    return null;
  }

  return (
    <UiMessageModal message={message} />
  );
});

const UiMessageModal = observer(function UiMessageModal(props: { message: IUiMessage }) {
  const {
    message,
  } = props;

  const UiMessageService = useService(IUiMessageService);

  const title = message.title || getTitle(message.type);
  const renderedMessage = useRenderedFormattedMessage(message);

  return (
    <Modal>
      <Modal.Header>{title}</Modal.Header>

      <Pad size="large">
        <TextBlock typographic userSelectable size="large">
          {renderedMessage}
        </TextBlock>
      </Pad>

      <Modal.Footer>
        <Flex repell>
          {!!message.extraActions?.length && (
            <ButtonBar>
              {message.extraActions.map((extraAction) => (
                <Button key={extraAction.id} text={extraAction.label} onClick={extraAction.action} />
              ))}
            </ButtonBar>
          )}

          <Button
            theme={message.extraActions?.length
              ? 'transparent'
              : 'default'}
            text={$L('#Servers_CloseOverlay')}
            onClick={UiMessageService.closeMessage}
          />
        </Flex>
      </Modal.Footer>
    </Modal>
  );
});

function getTitle(type: IUiMessage['type']) {
  switch (type) {
    case 'info':
      return $L('#Servers_Info');
    case 'warning':
      return $L('#Servers_Error');
    default:
      return undefined;
  }
}
