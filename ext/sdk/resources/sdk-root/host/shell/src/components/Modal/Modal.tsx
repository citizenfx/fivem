import React from 'react';
import ReactDOM from 'react-dom';

import s from './Modal.module.scss';


const noop = () => {};

export interface ModalProps {
  onClose?: () => void,
  children: React.ReactNode,
}

export const Modal = React.memo(({ children, onClose = noop }: ModalProps) => {
  const [modalOutlet, setModalOutlet] = React.useState<HTMLElement | null>(null);

  React.useLayoutEffect(() => {
    setModalOutlet(document.getElementById('modal-outlet'));
  }, []);

  React.useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      if (event.key === 'Escape') {
        onClose();
      }
    };

    window.addEventListener('keydown', handler);

    return () => window.removeEventListener('keydown', handler);
  }, [onClose]);

  if (modalOutlet) {
    return ReactDOM.createPortal(
      <div className={s.root}>
        <div className={s.backdrop} onClick={onClose}/>
        <div className={s.content}>
          {children}
        </div>
      </div>,
      modalOutlet,
    );
  }

  return null;
});
