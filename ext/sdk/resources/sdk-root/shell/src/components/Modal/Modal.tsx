import React from 'react';
import classnames from 'classnames';
import { attachOutlet } from 'utils/outlets';
import s from './Modal.module.scss';

const ModalOutlet = attachOutlet('modal-outlet');

const noop = () => { };

export interface ModalProps {
  onClose?: () => void,
  children: React.ReactNode,

  fullWidth?: boolean,
  fullHeight?: boolean,
}

export const Modal = React.memo(function Modal(props: ModalProps) {
  const {
    children,
    onClose = noop,
    fullWidth = false,
    fullHeight = false,
  } = props;

  React.useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      if (event.key === 'Escape') {
        onClose();
      }
    };

    window.addEventListener('keydown', handler);

    return () => window.removeEventListener('keydown', handler);
  }, [onClose]);

  const rootClassName = classnames(s.root, {
    [s['full-width']]: fullWidth,
    [s['full-height']]: fullHeight,
  });

  return (
    <ModalOutlet>
      <div className={rootClassName}>
        <div className={s.backdrop} onClick={onClose} />
        <div className={s.content}>
          {children}
        </div>
      </div>
    </ModalOutlet>
  );
});
