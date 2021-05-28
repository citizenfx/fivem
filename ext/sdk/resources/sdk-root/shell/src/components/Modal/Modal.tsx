import React from 'react';
import ReactDOM from 'react-dom';
import classnames from 'classnames';
import s from './Modal.module.scss';


const noop = () => {};

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
    const rootClassName = classnames(s.root, {
      [s['full-width']]: fullWidth,
      [s['full-height']]: fullHeight,
    });

    return ReactDOM.createPortal(
      <div className={rootClassName}>
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
