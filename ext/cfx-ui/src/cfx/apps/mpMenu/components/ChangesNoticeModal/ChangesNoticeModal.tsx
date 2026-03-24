import { Button, Flex, Icons, Modal, Pad, Symbols, Text } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useOpenFlag } from 'cfx/utils/hooks';

import { useAuthService } from '../../services/auth/auth.service';
import { useMpMenuServersConnectService } from '../../services/servers/serversConnect.mpMenu';

import s from './ChangesNoticeModal.module.scss';

const LS_KEY = 'changesNoticeDismissedVersion';
const VERSION = '1';

export const ChangesNoticeModal = observer(function ChangesNoticeModal() {
  const AuthService = useAuthService();
  const ServersConnectService = useMpMenuServersConnectService();

  const hasBeenDismissed = React.useMemo(() => {
    try {
      return window.localStorage.getItem(LS_KEY) === VERSION;
    } catch {
      return false;
    }
  }, []);

  const [modalOpen, _openModal, closeModal] = useOpenFlag(!hasBeenDismissed);

  const handleModalClose = React.useCallback(() => {
    try {
      window.localStorage.setItem(LS_KEY, VERSION);
    } catch {
      // ignore
    }

    closeModal();
  }, [closeModal]);

  // Do not show the notice modal when:
  // 1. AuthModal is active
  // 2. There is a pending server connection
  // 3. The user has already dismissed the notice
  if (AuthService.UIOpen || ServersConnectService.server !== null || !modalOpen) {
    return null;
  }

  return (
    <Modal onClose={handleModalClose}>
      <Pad size="xlarge">
        <Flex centered vertical gap="xlarge">
          <div className={s.icon}>
            {Icons.statusLevelMinor}
          </div>

          <Flex centered vertical gap="normal">
            <div className={s.title}>
              Notice
            </div>

            <Text typographic centered size="large" opacity="75">
              We are in the process of implementing new changes to the client.
              {' '}
              We are changing some colourways of our UI including updating server
              {' '}
              listing highlight colours and removing server name colour codes.
              <br />
              <br />
              For more information, please visit this
              {Symbols.nbsp}
              <a
                href="https://forum.cfx.re/t/5377566"
                target="_blank"
                rel="noreferrer"
              >
                Forum Post
              </a>
              .
            </Text>
          </Flex>

          <Button
            fullWidth
            onClick={handleModalClose}
            text="Continue"
            theme="primary"
            size="large"
          />
        </Flex>
      </Pad>
    </Modal>
  );
});
