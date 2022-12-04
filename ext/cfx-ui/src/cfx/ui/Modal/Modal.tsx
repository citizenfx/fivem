import React from "react";
import { Overlay } from "../Overlay/Overlay";
import { Button } from "../Button/Button";
import { Icons } from "../Icons";
import s from './Modal.module.scss';

export interface ModalProps {
  children: React.ReactNode,
  onClose?: () => void,

  backdropClassName?: string,
  contentClassName?: string,

  disableBackdropClose?: boolean,
}

export function Modal(props: ModalProps) {
  const {
    onClose,
    children,
    backdropClassName,
    contentClassName,
    disableBackdropClose = false,
  } = props;

  const handleBackdropClick = disableBackdropClose
    ? undefined
    : onClose;

  return (
    <Overlay>
      <Overlay.Backdrop onClick={handleBackdropClick} className={backdropClassName} />
      <Overlay.Content className={contentClassName}>
        <div className={s.root}>
          {!!onClose && (
            <div className={s.close}>
              <Button
                size="large"
                theme="transparent"
                icon={Icons.exit}
                onClick={onClose}
              />
            </div>
          )}

          {children}
        </div>
      </Overlay.Content>
    </Overlay>
  );
}

Modal.Header = (props: { children?: React.ReactNode }) => (
  <div className={s.header}>
    {props.children}
  </div>
);

Modal.Footer = (props: { children?: React.ReactNode }) => (
  <div className={s.footer}>
    {props.children}
  </div>
);
