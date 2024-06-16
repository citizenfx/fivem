import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './InfoPanel.module.scss';

export type InfoPanelType = 'default' | 'error' | 'success' | 'warning';

export type InfoPanelSize = 'auto' | 'small' | 'normal' | 'large';
export interface InfoPanelProps {
  type?: InfoPanelType;
  size?: InfoPanelSize;
  icon?: React.ReactNode;
  children?: React.ReactNode;
  className?: string;
  inline?: boolean;
}

export function InfoPanel(props: InfoPanelProps) {
  const {
    type = 'default',
    size = 'auto',
    icon, children,
    className,
    inline = false,
  } = props;

  const rootClassName = clsx(s.root, className, s[`type-${type}`], s[`size-${size}`], {
    [s.inline]: inline,
  });

  return (
    <div className={rootClassName}>
      {Boolean(icon) && (
        <div className={s.icon}>{icon}</div>
      )}

      <div className={s.content}>{children}</div>
    </div>
  );
}
