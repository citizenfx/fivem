import React from 'react';
import ReactFocusLock from 'react-focus-lock';

import { clsx } from 'cfx/utils/clsx';
import { attachOutlet } from 'cfx/utils/outlet';

import { Interactive } from '../Interactive/Interactive';

import s from './Overlay.module.scss';

const OverlayOutlet = attachOutlet('overlay-outlet');

export interface OverlayProps {
  children: React.ReactNode;
  className?: string;
}

export function Overlay(props: OverlayProps) {
  const {
    className,
    children,
  } = props;

  const rootClassName = clsx(s.root, className);

  return (
    <OverlayOutlet>
      <div className={rootClassName}>{children}</div>
    </OverlayOutlet>
  );
}

export interface OverlayBackdropProps {
  children?: React.ReactNode;

  className?: string;
  onClick?(): void;
}
Overlay.Backdrop = React.forwardRef(function OverlayBackdrop(
  props: OverlayBackdropProps,
  ref: React.Ref<HTMLDivElement>,
) {
  return (
    <Interactive ref={ref} onClick={props.onClick} className={clsx(s.backdrop, props.className)}>
      {props.children}
    </Interactive>
  );
});

export interface OverlayContentProps {
  children?: React.ReactNode;

  className?: string;
}
Overlay.Content = React.forwardRef(function OverlayContent(props: OverlayContentProps, ref: React.Ref<HTMLDivElement>) {
  return (
    <div ref={ref} className={clsx(s.content, props.className)}>
      <ReactFocusLock>{props.children}</ReactFocusLock>
    </div>
  );
});
