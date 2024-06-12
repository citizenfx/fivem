import React from 'react';

import { noop } from 'cfx/utils/functional';
import { useKeyboardClose } from 'cfx/utils/hooks';

import { Button } from '../Button/Button';
import { Icons } from '../Icons';
import { Overlay } from '../Overlay/Overlay';

import s from './Modal.module.scss';

export interface ModalProps {
  children: React.ReactNode;
  onClose?: () => void;

  backdropClassName?: string;
  contentClassName?: string;

  disableBackdropClose?: boolean;
}

export function Modal(props: ModalProps) {
  const {
    onClose = noop,
    children,
    backdropClassName,
    contentClassName,
    disableBackdropClose = false,
  } = props;

  const handleBackdropClick = disableBackdropClose
    ? noop
    : onClose;

  useKeyboardClose(handleBackdropClick);

  return (
    <Overlay>
      <Overlay.Backdrop onClick={handleBackdropClick} className={backdropClassName} />
      <Overlay.Content className={contentClassName}>
        <div className={s.root}>
          {!!onClose && (
            <div className={s.close}>
              <Button size="large" theme="transparent" icon={Icons.exit} onClick={onClose} />
            </div>
          )}

          {children}
        </div>
      </Overlay.Content>
    </Overlay>
  );
}

Modal.Header = function ModalHeader({
  children,
}: { children?: React.ReactNode }) {
  return (
    <div className={s.header}>{children}</div>
  );
};

Modal.Footer = function ModalFooter({
  children,
}: { children?: React.ReactNode }) {
  return (
    <div className={s.footer}>{children}</div>
  );
};
