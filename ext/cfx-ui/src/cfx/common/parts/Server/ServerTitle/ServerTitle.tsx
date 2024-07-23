import { clsx } from '@cfx-dev/ui-components';
import React from 'react';

import { colorize } from 'cfx/utils/colorize';

import s from './ServerTitle.module.scss';

export interface ServerTitleProps {
  title: string;
  size?: 'small' | 'normal' | 'large' | 'xlarge' | 'xxlarge' | 'xxxlarge';
  truncated?: boolean;
}

const cached: Record<string, React.ReactNode> = {};

export function ServerTitle(props: ServerTitleProps) {
  const {
    title,
    size = 'large',
    truncated = false,
  } = props;

  if (!cached[title]) {
    cached[title] = colorize(title);
  }

  const rootClassName = clsx(s.root, s[`size-${size}`], {
    [s.truncated]: truncated,
  });

  return (
    <span className={rootClassName}>{cached[title]}</span>
  );
}
